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

CC = egcs
#GNUINC = /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3_U1/2.6.3/include
#GNUINC = /usr/lib/gcc-lib/i486-linux/2.5.8/include
#GNUINC = /usr/lib/gcc-lib/i486-linux/2.6.3/include
#GNUINC = /usr/lib/gcc-lib/i486-linux/2.7.0/include
#GNUINC = /usr/lib/gcc-lib/i486-linux/2.7.2/include
#GNUINC = /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include
GNUINC = /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include
#PROF = -pg
#BUGFLAGS = -DDEBUG_FILE
#WARN = -Wall -Wstrict-prototypes -Wmissing-prototypes
WARN = -Wall -Wmissing-prototypes
#WARN = -w
#DEBUG = -fomit-frame-pointer
DEBUG = -g3 -static
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
LIB =	-L$(IDENTDIR) -lident -lcrypt
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
obj/act_comm.o: /usr/include/features.h /usr/include/sys/cdefs.h
obj/act_comm.o: /usr/include/gnu/stubs.h /usr/include/_G_config.h
obj/act_comm.o: /usr/include/gnu/types.h
obj/act_comm.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stddef.h
obj/act_comm.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stdarg.h
obj/act_comm.o: /usr/include/stdio_lim.h /usr/include/stdlib.h
obj/act_comm.o: /usr/include/sys/types.h /usr/include/time.h
obj/act_comm.o: /usr/include/endian.h /usr/include/bytesex.h
obj/act_comm.o: /usr/include/sys/select.h /usr/include/selectbits.h
obj/act_comm.o: /usr/include/alloca.h /usr/include/unistd.h
obj/act_comm.o: /usr/include/posix_opt.h /usr/include/confname.h
obj/act_comm.o: /usr/include/string.h include/global.h include/bug.h
obj/act_comm.o: include/utils.h include/comm.h include/interpreter.h
obj/act_comm.o: include/handler.h include/db.h include/spells.h
obj/act_comm.o: include/constants.h include/limits.h include/multiclass.h
obj/act_comm.o: include/act_comm.h
obj/act_info.o: /usr/include/stdio.h /usr/include/libio.h
obj/act_info.o: /usr/include/features.h /usr/include/sys/cdefs.h
obj/act_info.o: /usr/include/gnu/stubs.h /usr/include/_G_config.h
obj/act_info.o: /usr/include/gnu/types.h
obj/act_info.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stddef.h
obj/act_info.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stdarg.h
obj/act_info.o: /usr/include/stdio_lim.h /usr/include/stdlib.h
obj/act_info.o: /usr/include/sys/types.h /usr/include/time.h
obj/act_info.o: /usr/include/endian.h /usr/include/bytesex.h
obj/act_info.o: /usr/include/sys/select.h /usr/include/selectbits.h
obj/act_info.o: /usr/include/alloca.h /usr/include/unistd.h
obj/act_info.o: /usr/include/posix_opt.h /usr/include/confname.h
obj/act_info.o: /usr/include/string.h /usr/include/ctype.h include/global.h
obj/act_info.o: include/bug.h include/version.h include/utils.h
obj/act_info.o: include/comm.h include/interpreter.h include/handler.h
obj/act_info.o: include/db.h include/spells.h include/limits.h include/trap.h
obj/act_info.o: include/hash.h include/constants.h include/spell_parser.h
obj/act_info.o: include/whod.h include/multiclass.h include/modify.h
obj/act_info.o: include/act_wiz.h include/act_skills.h include/spec_procs.h
obj/act_info.o: include/tracking.h include/act_info.h
obj/act_move.o: /usr/include/stdio.h /usr/include/libio.h
obj/act_move.o: /usr/include/features.h /usr/include/sys/cdefs.h
obj/act_move.o: /usr/include/gnu/stubs.h /usr/include/_G_config.h
obj/act_move.o: /usr/include/gnu/types.h
obj/act_move.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stddef.h
obj/act_move.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stdarg.h
obj/act_move.o: /usr/include/stdio_lim.h /usr/include/stdlib.h
obj/act_move.o: /usr/include/sys/types.h /usr/include/time.h
obj/act_move.o: /usr/include/endian.h /usr/include/bytesex.h
obj/act_move.o: /usr/include/sys/select.h /usr/include/selectbits.h
obj/act_move.o: /usr/include/alloca.h /usr/include/unistd.h
obj/act_move.o: /usr/include/posix_opt.h /usr/include/confname.h
obj/act_move.o: /usr/include/string.h include/global.h include/bug.h
obj/act_move.o: include/utils.h include/comm.h include/interpreter.h
obj/act_move.o: include/handler.h include/db.h include/spells.h
obj/act_move.o: include/trap.h include/constants.h include/whod.h
obj/act_move.o: include/multiclass.h include/fight.h include/reception.h
obj/act_move.o: include/magic_utils.h include/spell_parser.h
obj/act_move.o: include/act_info.h include/act_skills.h include/act_move.h
obj/act_obj.o: /usr/include/stdio.h /usr/include/libio.h
obj/act_obj.o: /usr/include/features.h /usr/include/sys/cdefs.h
obj/act_obj.o: /usr/include/gnu/stubs.h /usr/include/_G_config.h
obj/act_obj.o: /usr/include/gnu/types.h
obj/act_obj.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stddef.h
obj/act_obj.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stdarg.h
obj/act_obj.o: /usr/include/stdio_lim.h /usr/include/stdlib.h
obj/act_obj.o: /usr/include/sys/types.h /usr/include/time.h
obj/act_obj.o: /usr/include/endian.h /usr/include/bytesex.h
obj/act_obj.o: /usr/include/sys/select.h /usr/include/selectbits.h
obj/act_obj.o: /usr/include/alloca.h /usr/include/unistd.h
obj/act_obj.o: /usr/include/posix_opt.h /usr/include/confname.h
obj/act_obj.o: /usr/include/string.h /usr/include/assert.h
obj/act_obj.o: /usr/include/ctype.h include/global.h include/bug.h
obj/act_obj.o: include/utils.h include/comm.h include/interpreter.h
obj/act_obj.o: include/handler.h include/db.h include/spells.h include/trap.h
obj/act_obj.o: include/constants.h include/spell_parser.h
obj/act_obj.o: include/multiclass.h include/limits.h include/fight.h
obj/act_obj.o: include/act_info.h include/act_obj.h
obj/act_off.o: /usr/include/stdio.h /usr/include/libio.h
obj/act_off.o: /usr/include/features.h /usr/include/sys/cdefs.h
obj/act_off.o: /usr/include/gnu/stubs.h /usr/include/_G_config.h
obj/act_off.o: /usr/include/gnu/types.h
obj/act_off.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stddef.h
obj/act_off.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stdarg.h
obj/act_off.o: /usr/include/stdio_lim.h /usr/include/stdlib.h
obj/act_off.o: /usr/include/sys/types.h /usr/include/time.h
obj/act_off.o: /usr/include/endian.h /usr/include/bytesex.h
obj/act_off.o: /usr/include/sys/select.h /usr/include/selectbits.h
obj/act_off.o: /usr/include/alloca.h /usr/include/unistd.h
obj/act_off.o: /usr/include/posix_opt.h /usr/include/confname.h
obj/act_off.o: /usr/include/string.h /usr/include/ctype.h include/global.h
obj/act_off.o: include/bug.h include/utils.h include/comm.h
obj/act_off.o: include/interpreter.h include/handler.h include/db.h
obj/act_off.o: include/spells.h include/limits.h include/opinion.h
obj/act_off.o: include/multiclass.h include/constants.h include/spec_procs.h
obj/act_off.o: include/fight.h include/act_skills.h include/act_move.h
obj/act_off.o: include/spell_parser.h include/act_info.h
obj/act_off.o: include/breath_weapons.h include/act_off.h
obj/act_other.o: /usr/include/stdio.h /usr/include/libio.h
obj/act_other.o: /usr/include/features.h /usr/include/sys/cdefs.h
obj/act_other.o: /usr/include/gnu/stubs.h /usr/include/_G_config.h
obj/act_other.o: /usr/include/gnu/types.h
obj/act_other.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stddef.h
obj/act_other.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stdarg.h
obj/act_other.o: /usr/include/stdio_lim.h /usr/include/stdlib.h
obj/act_other.o: /usr/include/sys/types.h /usr/include/time.h
obj/act_other.o: /usr/include/endian.h /usr/include/bytesex.h
obj/act_other.o: /usr/include/sys/select.h /usr/include/selectbits.h
obj/act_other.o: /usr/include/alloca.h /usr/include/unistd.h
obj/act_other.o: /usr/include/posix_opt.h /usr/include/confname.h
obj/act_other.o: /usr/include/string.h /usr/include/ctype.h include/global.h
obj/act_other.o: include/bug.h include/utils.h include/comm.h
obj/act_other.o: include/interpreter.h include/handler.h include/db.h
obj/act_other.o: include/spells.h include/limits.h include/constants.h
obj/act_other.o: include/spell_parser.h include/reception.h
obj/act_other.o: include/multiclass.h include/fight.h include/spec_procs.h
obj/act_other.o: include/act_skills.h include/act_comm.h include/modify.h
obj/act_other.o: include/act_other.h
obj/act_skills.o: /usr/include/stdio.h /usr/include/libio.h
obj/act_skills.o: /usr/include/features.h /usr/include/sys/cdefs.h
obj/act_skills.o: /usr/include/gnu/stubs.h /usr/include/_G_config.h
obj/act_skills.o: /usr/include/gnu/types.h
obj/act_skills.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stddef.h
obj/act_skills.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stdarg.h
obj/act_skills.o: /usr/include/stdio_lim.h /usr/include/stdlib.h
obj/act_skills.o: /usr/include/sys/types.h /usr/include/time.h
obj/act_skills.o: /usr/include/endian.h /usr/include/bytesex.h
obj/act_skills.o: /usr/include/sys/select.h /usr/include/selectbits.h
obj/act_skills.o: /usr/include/alloca.h /usr/include/unistd.h
obj/act_skills.o: /usr/include/posix_opt.h /usr/include/confname.h
obj/act_skills.o: include/global.h include/bug.h include/utils.h
obj/act_skills.o: include/comm.h include/constants.h include/db.h
obj/act_skills.o: include/fight.h include/handler.h include/interpreter.h
obj/act_skills.o: include/multiclass.h include/spell_parser.h
obj/act_skills.o: include/spells.h include/spec_procs.h include/act_skills.h
obj/act_social.o: /usr/include/stdio.h /usr/include/libio.h
obj/act_social.o: /usr/include/features.h /usr/include/sys/cdefs.h
obj/act_social.o: /usr/include/gnu/stubs.h /usr/include/_G_config.h
obj/act_social.o: /usr/include/gnu/types.h
obj/act_social.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stddef.h
obj/act_social.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stdarg.h
obj/act_social.o: /usr/include/stdio_lim.h /usr/include/stdlib.h
obj/act_social.o: /usr/include/sys/types.h /usr/include/time.h
obj/act_social.o: /usr/include/endian.h /usr/include/bytesex.h
obj/act_social.o: /usr/include/sys/select.h /usr/include/selectbits.h
obj/act_social.o: /usr/include/alloca.h /usr/include/unistd.h
obj/act_social.o: /usr/include/posix_opt.h /usr/include/confname.h
obj/act_social.o: /usr/include/string.h include/global.h include/bug.h
obj/act_social.o: include/utils.h include/comm.h include/interpreter.h
obj/act_social.o: include/handler.h include/db.h include/spells.h
obj/act_social.o: include/multiclass.h include/act_social.h
obj/act_wiz.o: /usr/include/stdio.h /usr/include/libio.h
obj/act_wiz.o: /usr/include/features.h /usr/include/sys/cdefs.h
obj/act_wiz.o: /usr/include/gnu/stubs.h /usr/include/_G_config.h
obj/act_wiz.o: /usr/include/gnu/types.h
obj/act_wiz.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stddef.h
obj/act_wiz.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stdarg.h
obj/act_wiz.o: /usr/include/stdio_lim.h /usr/include/stdlib.h
obj/act_wiz.o: /usr/include/sys/types.h /usr/include/time.h
obj/act_wiz.o: /usr/include/endian.h /usr/include/bytesex.h
obj/act_wiz.o: /usr/include/sys/select.h /usr/include/selectbits.h
obj/act_wiz.o: /usr/include/alloca.h /usr/include/unistd.h
obj/act_wiz.o: /usr/include/posix_opt.h /usr/include/confname.h
obj/act_wiz.o: /usr/include/string.h /usr/include/ctype.h include/global.h
obj/act_wiz.o: include/bug.h include/utils.h include/comm.h
obj/act_wiz.o: include/interpreter.h include/handler.h include/db.h
obj/act_wiz.o: include/spells.h include/limits.h include/constants.h
obj/act_wiz.o: include/spell_parser.h include/board.h include/whod.h
obj/act_wiz.o: include/reception.h include/spec_procs.h include/multiclass.h
obj/act_wiz.o: include/act_skills.h include/act_info.h include/fight.h
obj/act_wiz.o: include/hash.h include/weather.h include/modify.h
obj/act_wiz.o: include/act_wiz.h
obj/board.o: /usr/include/stdio.h /usr/include/libio.h
obj/board.o: /usr/include/features.h /usr/include/sys/cdefs.h
obj/board.o: /usr/include/gnu/stubs.h /usr/include/_G_config.h
obj/board.o: /usr/include/gnu/types.h
obj/board.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stddef.h
obj/board.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stdarg.h
obj/board.o: /usr/include/stdio_lim.h /usr/include/stdlib.h
obj/board.o: /usr/include/sys/types.h /usr/include/time.h
obj/board.o: /usr/include/endian.h /usr/include/bytesex.h
obj/board.o: /usr/include/sys/select.h /usr/include/selectbits.h
obj/board.o: /usr/include/alloca.h /usr/include/unistd.h
obj/board.o: /usr/include/posix_opt.h /usr/include/confname.h
obj/board.o: /usr/include/string.h /usr/include/ctype.h
obj/board.o: /usr/include/sys/time.h /usr/include/timebits.h include/global.h
obj/board.o: include/bug.h include/utils.h include/comm.h include/db.h
obj/board.o: include/interpreter.h include/multiclass.h include/modify.h
obj/board.o: include/handler.h include/act_info.h include/board.h
obj/bug.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
obj/bug.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
obj/bug.o: /usr/include/_G_config.h /usr/include/gnu/types.h
obj/bug.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stddef.h
obj/bug.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stdarg.h
obj/bug.o: /usr/include/stdio_lim.h /usr/include/stdlib.h
obj/bug.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/endian.h
obj/bug.o: /usr/include/bytesex.h /usr/include/sys/select.h
obj/bug.o: /usr/include/selectbits.h /usr/include/alloca.h
obj/bug.o: /usr/include/unistd.h /usr/include/posix_opt.h
obj/bug.o: /usr/include/confname.h /usr/include/string.h
obj/bug.o: /usr/include/sys/timeb.h include/global.h include/utils.h
obj/bug.o: include/comm.h include/multiclass.h include/bug.h
obj/comm.o: /usr/include/errno.h /usr/include/features.h
obj/comm.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
obj/comm.o: /usr/include/errnos.h /usr/include/linux/errno.h
obj/comm.o: /usr/include/asm/errno.h /usr/include/stdio.h
obj/comm.o: /usr/include/libio.h /usr/include/_G_config.h
obj/comm.o: /usr/include/gnu/types.h
obj/comm.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stddef.h
obj/comm.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stdarg.h
obj/comm.o: /usr/include/stdio_lim.h /usr/include/stdlib.h
obj/comm.o: /usr/include/sys/types.h /usr/include/time.h
obj/comm.o: /usr/include/endian.h /usr/include/bytesex.h
obj/comm.o: /usr/include/sys/select.h /usr/include/selectbits.h
obj/comm.o: /usr/include/alloca.h /usr/include/unistd.h
obj/comm.o: /usr/include/posix_opt.h /usr/include/confname.h
obj/comm.o: /usr/include/ctype.h /usr/include/sys/socket.h
obj/comm.o: /usr/include/socketbits.h /usr/include/sockaddrcom.h
obj/comm.o: /usr/include/asm/socket.h /usr/include/asm/sockios.h
obj/comm.o: /usr/include/sys/wait.h /usr/include/waitflags.h
obj/comm.o: /usr/include/waitstatus.h /usr/include/netinet/in.h
obj/comm.o: /usr/include/netdb.h /usr/include/rpc/netdb.h
obj/comm.o: /usr/include/string.h /usr/include/sys/time.h
obj/comm.o: /usr/include/timebits.h /usr/include/fcntl.h
obj/comm.o: /usr/include/fcntlbits.h /usr/include/signal.h
obj/comm.o: /usr/include/sigset.h /usr/include/signum.h
obj/comm.o: /usr/include/sigaction.h /usr/include/sigcontext.h
obj/comm.o: /usr/include/asm/sigcontext.h /usr/include/sys/un.h
obj/comm.o: /usr/include/sys/resource.h /usr/include/resourcebits.h
obj/comm.o: /usr/include/asm/resource.h include/global.h include/bug.h
obj/comm.o: include/utils.h include/interpreter.h include/handler.h
obj/comm.o: include/db.h include/modify.h include/whod.h include/multiclass.h
obj/comm.o: include/weather.h include/limits.h include/spell_parser.h
obj/comm.o: include/sound.h include/fight.h include/mob_actions.h
obj/comm.o: include/comm.h libident-0.19/ident.h
obj/constants.o: /usr/include/stdlib.h /usr/include/features.h
obj/constants.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
obj/constants.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stddef.h
obj/constants.o: /usr/include/sys/types.h /usr/include/gnu/types.h
obj/constants.o: /usr/include/time.h /usr/include/endian.h
obj/constants.o: /usr/include/bytesex.h /usr/include/sys/select.h
obj/constants.o: /usr/include/selectbits.h /usr/include/alloca.h
obj/constants.o: /usr/include/unistd.h /usr/include/posix_opt.h
obj/constants.o: /usr/include/confname.h include/global.h include/bug.h
obj/constants.o: include/limits.h include/trap.h include/constants.h
obj/db.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
obj/db.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
obj/db.o: /usr/include/_G_config.h /usr/include/gnu/types.h
obj/db.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stddef.h
obj/db.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stdarg.h
obj/db.o: /usr/include/stdio_lim.h /usr/include/stdlib.h
obj/db.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/endian.h
obj/db.o: /usr/include/bytesex.h /usr/include/sys/select.h
obj/db.o: /usr/include/selectbits.h /usr/include/alloca.h
obj/db.o: /usr/include/unistd.h /usr/include/posix_opt.h
obj/db.o: /usr/include/confname.h /usr/include/string.h /usr/include/ctype.h
obj/db.o: include/global.h include/bug.h include/utils.h include/comm.h
obj/db.o: include/handler.h include/limits.h include/opinion.h include/hash.h
obj/db.o: include/constants.h include/spells.h include/spell_parser.h
obj/db.o: include/reception.h include/weather.h include/modify.h
obj/db.o: include/fight.h include/act_social.h include/spec_procs.h
obj/db.o: include/multiclass.h include/board.h include/interpreter.h
obj/db.o: include/db.h
obj/fight.o: /usr/include/stdio.h /usr/include/libio.h
obj/fight.o: /usr/include/features.h /usr/include/sys/cdefs.h
obj/fight.o: /usr/include/gnu/stubs.h /usr/include/_G_config.h
obj/fight.o: /usr/include/gnu/types.h
obj/fight.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stddef.h
obj/fight.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stdarg.h
obj/fight.o: /usr/include/stdio_lim.h /usr/include/stdlib.h
obj/fight.o: /usr/include/sys/types.h /usr/include/time.h
obj/fight.o: /usr/include/endian.h /usr/include/bytesex.h
obj/fight.o: /usr/include/sys/select.h /usr/include/selectbits.h
obj/fight.o: /usr/include/alloca.h /usr/include/unistd.h
obj/fight.o: /usr/include/posix_opt.h /usr/include/confname.h
obj/fight.o: /usr/include/string.h /usr/include/assert.h include/global.h
obj/fight.o: include/bug.h include/utils.h include/comm.h include/handler.h
obj/fight.o: include/interpreter.h include/db.h include/constants.h
obj/fight.o: include/spells.h include/spell_parser.h include/limits.h
obj/fight.o: include/random.h include/act_move.h include/reception.h
obj/fight.o: include/multiclass.h include/act_wiz.h include/act_skills.h
obj/fight.o: include/opinion.h include/spec_procs.h include/mob_actions.h
obj/fight.o: include/act_off.h include/fight.h
obj/handler.o: /usr/include/stdio.h /usr/include/libio.h
obj/handler.o: /usr/include/features.h /usr/include/sys/cdefs.h
obj/handler.o: /usr/include/gnu/stubs.h /usr/include/_G_config.h
obj/handler.o: /usr/include/gnu/types.h
obj/handler.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stddef.h
obj/handler.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stdarg.h
obj/handler.o: /usr/include/stdio_lim.h /usr/include/stdlib.h
obj/handler.o: /usr/include/sys/types.h /usr/include/time.h
obj/handler.o: /usr/include/endian.h /usr/include/bytesex.h
obj/handler.o: /usr/include/sys/select.h /usr/include/selectbits.h
obj/handler.o: /usr/include/alloca.h /usr/include/unistd.h
obj/handler.o: /usr/include/posix_opt.h /usr/include/confname.h
obj/handler.o: /usr/include/string.h /usr/include/ctype.h
obj/handler.o: /usr/include/signal.h /usr/include/sigset.h
obj/handler.o: /usr/include/signum.h /usr/include/sigaction.h
obj/handler.o: /usr/include/sigcontext.h /usr/include/asm/sigcontext.h
obj/handler.o: include/global.h include/bug.h include/utils.h include/comm.h
obj/handler.o: include/db.h include/interpreter.h include/spells.h
obj/handler.o: include/spell_parser.h include/constants.h include/fight.h
obj/handler.o: include/modify.h include/multiclass.h include/opinion.h
obj/handler.o: include/handler.h
obj/hash.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
obj/hash.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
obj/hash.o: /usr/include/_G_config.h /usr/include/gnu/types.h
obj/hash.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stddef.h
obj/hash.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stdarg.h
obj/hash.o: /usr/include/stdio_lim.h /usr/include/stdlib.h
obj/hash.o: /usr/include/sys/types.h /usr/include/time.h
obj/hash.o: /usr/include/endian.h /usr/include/bytesex.h
obj/hash.o: /usr/include/sys/select.h /usr/include/selectbits.h
obj/hash.o: /usr/include/alloca.h /usr/include/unistd.h
obj/hash.o: /usr/include/posix_opt.h /usr/include/confname.h
obj/hash.o: /usr/include/string.h include/global.h include/bug.h
obj/hash.o: include/comm.h include/hash.h
obj/interpreter.o: /usr/include/stdio.h /usr/include/libio.h
obj/interpreter.o: /usr/include/features.h /usr/include/sys/cdefs.h
obj/interpreter.o: /usr/include/gnu/stubs.h /usr/include/_G_config.h
obj/interpreter.o: /usr/include/gnu/types.h
obj/interpreter.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stddef.h
obj/interpreter.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stdarg.h
obj/interpreter.o: /usr/include/stdio_lim.h /usr/include/stdlib.h
obj/interpreter.o: /usr/include/sys/types.h /usr/include/time.h
obj/interpreter.o: /usr/include/endian.h /usr/include/bytesex.h
obj/interpreter.o: /usr/include/sys/select.h /usr/include/selectbits.h
obj/interpreter.o: /usr/include/alloca.h /usr/include/unistd.h
obj/interpreter.o: /usr/include/posix_opt.h /usr/include/confname.h
obj/interpreter.o: /usr/include/string.h /usr/include/ctype.h
obj/interpreter.o: /usr/include/arpa/telnet.h /usr/include/sys/timeb.h
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
obj/limits.o: /usr/include/features.h /usr/include/sys/cdefs.h
obj/limits.o: /usr/include/gnu/stubs.h /usr/include/_G_config.h
obj/limits.o: /usr/include/gnu/types.h
obj/limits.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stddef.h
obj/limits.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stdarg.h
obj/limits.o: /usr/include/stdio_lim.h /usr/include/stdlib.h
obj/limits.o: /usr/include/sys/types.h /usr/include/time.h
obj/limits.o: /usr/include/endian.h /usr/include/bytesex.h
obj/limits.o: /usr/include/sys/select.h /usr/include/selectbits.h
obj/limits.o: /usr/include/alloca.h /usr/include/unistd.h
obj/limits.o: /usr/include/posix_opt.h /usr/include/confname.h
obj/limits.o: /usr/include/ctype.h /usr/include/string.h
obj/limits.o: /usr/include/assert.h include/global.h include/bug.h
obj/limits.o: include/spells.h include/comm.h include/db.h
obj/limits.o: include/spell_parser.h include/constants.h include/utils.h
obj/limits.o: include/multiclass.h include/fight.h include/reception.h
obj/limits.o: include/interpreter.h include/handler.h include/act_obj.h
obj/limits.o: include/limits.h
obj/magic.o: /usr/include/stdio.h /usr/include/libio.h
obj/magic.o: /usr/include/features.h /usr/include/sys/cdefs.h
obj/magic.o: /usr/include/gnu/stubs.h /usr/include/_G_config.h
obj/magic.o: /usr/include/gnu/types.h
obj/magic.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stddef.h
obj/magic.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stdarg.h
obj/magic.o: /usr/include/stdio_lim.h /usr/include/stdlib.h
obj/magic.o: /usr/include/sys/types.h /usr/include/time.h
obj/magic.o: /usr/include/endian.h /usr/include/bytesex.h
obj/magic.o: /usr/include/sys/select.h /usr/include/selectbits.h
obj/magic.o: /usr/include/alloca.h /usr/include/unistd.h
obj/magic.o: /usr/include/posix_opt.h /usr/include/confname.h
obj/magic.o: /usr/include/string.h /usr/include/assert.h include/global.h
obj/magic.o: include/bug.h include/utils.h include/comm.h include/handler.h
obj/magic.o: include/limits.h include/db.h include/constants.h
obj/magic.o: include/spells.h include/spell_parser.h include/multiclass.h
obj/magic.o: include/fight.h include/opinion.h include/reception.h
obj/magic.o: include/magic_utils.h include/act_off.h include/act_obj.h
obj/magic.o: include/act_info.h
obj/magic_utils.o: /usr/include/stdio.h /usr/include/libio.h
obj/magic_utils.o: /usr/include/features.h /usr/include/sys/cdefs.h
obj/magic_utils.o: /usr/include/gnu/stubs.h /usr/include/_G_config.h
obj/magic_utils.o: /usr/include/gnu/types.h
obj/magic_utils.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stddef.h
obj/magic_utils.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stdarg.h
obj/magic_utils.o: /usr/include/stdio_lim.h /usr/include/stdlib.h
obj/magic_utils.o: /usr/include/sys/types.h /usr/include/time.h
obj/magic_utils.o: /usr/include/endian.h /usr/include/bytesex.h
obj/magic_utils.o: /usr/include/sys/select.h /usr/include/selectbits.h
obj/magic_utils.o: /usr/include/alloca.h /usr/include/unistd.h
obj/magic_utils.o: /usr/include/posix_opt.h /usr/include/confname.h
obj/magic_utils.o: /usr/include/assert.h include/global.h include/bug.h
obj/magic_utils.o: include/utils.h include/comm.h include/db.h
obj/magic_utils.o: include/spells.h include/handler.h include/limits.h
obj/magic_utils.o: include/fight.h include/spell_parser.h
obj/magic_utils.o: include/multiclass.h include/magic_utils.h
obj/modify.o: /usr/include/stdio.h /usr/include/libio.h
obj/modify.o: /usr/include/features.h /usr/include/sys/cdefs.h
obj/modify.o: /usr/include/gnu/stubs.h /usr/include/_G_config.h
obj/modify.o: /usr/include/gnu/types.h
obj/modify.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stddef.h
obj/modify.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stdarg.h
obj/modify.o: /usr/include/stdio_lim.h /usr/include/stdlib.h
obj/modify.o: /usr/include/sys/types.h /usr/include/time.h
obj/modify.o: /usr/include/endian.h /usr/include/bytesex.h
obj/modify.o: /usr/include/sys/select.h /usr/include/selectbits.h
obj/modify.o: /usr/include/alloca.h /usr/include/unistd.h
obj/modify.o: /usr/include/posix_opt.h /usr/include/confname.h
obj/modify.o: /usr/include/ctype.h /usr/include/string.h include/global.h
obj/modify.o: include/bug.h include/utils.h include/interpreter.h
obj/modify.o: include/handler.h include/db.h include/comm.h
obj/modify.o: include/multiclass.h include/modify.h
obj/multiclass.o: /usr/include/stdio.h /usr/include/libio.h
obj/multiclass.o: /usr/include/features.h /usr/include/sys/cdefs.h
obj/multiclass.o: /usr/include/gnu/stubs.h /usr/include/_G_config.h
obj/multiclass.o: /usr/include/gnu/types.h
obj/multiclass.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stddef.h
obj/multiclass.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stdarg.h
obj/multiclass.o: /usr/include/stdio_lim.h /usr/include/stdlib.h
obj/multiclass.o: /usr/include/sys/types.h /usr/include/time.h
obj/multiclass.o: /usr/include/endian.h /usr/include/bytesex.h
obj/multiclass.o: /usr/include/sys/select.h /usr/include/selectbits.h
obj/multiclass.o: /usr/include/alloca.h /usr/include/unistd.h
obj/multiclass.o: /usr/include/posix_opt.h /usr/include/confname.h
obj/multiclass.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/limits.h
obj/multiclass.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/syslimits.h
obj/multiclass.o: /usr/include/string.h include/global.h include/bug.h
obj/multiclass.o: include/utils.h include/comm.h include/db.h
obj/multiclass.o: include/interpreter.h include/handler.h include/spells.h
obj/multiclass.o: include/limits.h include/opinion.h include/constants.h
obj/multiclass.o: include/multiclass.h
obj/opinion.o: /usr/include/stdio.h /usr/include/libio.h
obj/opinion.o: /usr/include/features.h /usr/include/sys/cdefs.h
obj/opinion.o: /usr/include/gnu/stubs.h /usr/include/_G_config.h
obj/opinion.o: /usr/include/gnu/types.h
obj/opinion.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stddef.h
obj/opinion.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stdarg.h
obj/opinion.o: /usr/include/stdio_lim.h /usr/include/stdlib.h
obj/opinion.o: /usr/include/sys/types.h /usr/include/time.h
obj/opinion.o: /usr/include/endian.h /usr/include/bytesex.h
obj/opinion.o: /usr/include/sys/select.h /usr/include/selectbits.h
obj/opinion.o: /usr/include/alloca.h /usr/include/unistd.h
obj/opinion.o: /usr/include/posix_opt.h /usr/include/confname.h
obj/opinion.o: /usr/include/string.h /usr/include/ctype.h include/global.h
obj/opinion.o: include/bug.h include/utils.h include/spells.h
obj/opinion.o: include/constants.h include/comm.h include/db.h
obj/opinion.o: include/multiclass.h include/opinion.h
obj/random.o: /usr/include/stdio.h /usr/include/libio.h
obj/random.o: /usr/include/features.h /usr/include/sys/cdefs.h
obj/random.o: /usr/include/gnu/stubs.h /usr/include/_G_config.h
obj/random.o: /usr/include/gnu/types.h
obj/random.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stddef.h
obj/random.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stdarg.h
obj/random.o: /usr/include/stdio_lim.h /usr/include/stdlib.h
obj/random.o: /usr/include/sys/types.h /usr/include/time.h
obj/random.o: /usr/include/endian.h /usr/include/bytesex.h
obj/random.o: /usr/include/sys/select.h /usr/include/selectbits.h
obj/random.o: /usr/include/alloca.h /usr/include/unistd.h
obj/random.o: /usr/include/posix_opt.h /usr/include/confname.h
obj/random.o: /usr/include/string.h /usr/include/sys/timeb.h include/global.h
obj/random.o: include/utils.h include/comm.h include/random.h
obj/reception.o: /usr/include/stdio.h /usr/include/libio.h
obj/reception.o: /usr/include/features.h /usr/include/sys/cdefs.h
obj/reception.o: /usr/include/gnu/stubs.h /usr/include/_G_config.h
obj/reception.o: /usr/include/gnu/types.h
obj/reception.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stddef.h
obj/reception.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stdarg.h
obj/reception.o: /usr/include/stdio_lim.h /usr/include/stdlib.h
obj/reception.o: /usr/include/sys/types.h /usr/include/time.h
obj/reception.o: /usr/include/endian.h /usr/include/bytesex.h
obj/reception.o: /usr/include/sys/select.h /usr/include/selectbits.h
obj/reception.o: /usr/include/alloca.h /usr/include/unistd.h
obj/reception.o: /usr/include/posix_opt.h /usr/include/confname.h
obj/reception.o: /usr/include/sys/time.h /usr/include/timebits.h
obj/reception.o: /usr/include/ctype.h /usr/include/signal.h
obj/reception.o: /usr/include/sigset.h /usr/include/signum.h
obj/reception.o: /usr/include/sigaction.h /usr/include/sigcontext.h
obj/reception.o: /usr/include/asm/sigcontext.h include/global.h include/bug.h
obj/reception.o: include/comm.h include/handler.h include/db.h
obj/reception.o: include/interpreter.h include/utils.h include/spells.h
obj/reception.o: include/multiclass.h include/reception.h
obj/shop.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
obj/shop.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
obj/shop.o: /usr/include/_G_config.h /usr/include/gnu/types.h
obj/shop.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stddef.h
obj/shop.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stdarg.h
obj/shop.o: /usr/include/stdio_lim.h /usr/include/stdlib.h
obj/shop.o: /usr/include/sys/types.h /usr/include/time.h
obj/shop.o: /usr/include/endian.h /usr/include/bytesex.h
obj/shop.o: /usr/include/sys/select.h /usr/include/selectbits.h
obj/shop.o: /usr/include/alloca.h /usr/include/unistd.h
obj/shop.o: /usr/include/posix_opt.h /usr/include/confname.h
obj/shop.o: /usr/include/string.h include/global.h include/bug.h
obj/shop.o: include/comm.h include/handler.h include/db.h
obj/shop.o: include/interpreter.h include/utils.h include/constants.h
obj/shop.o: include/act_comm.h include/multiclass.h include/act_social.h
obj/shop.o: include/act_wiz.h include/shop.h
obj/signals.o: /usr/include/stdio.h /usr/include/libio.h
obj/signals.o: /usr/include/features.h /usr/include/sys/cdefs.h
obj/signals.o: /usr/include/gnu/stubs.h /usr/include/_G_config.h
obj/signals.o: /usr/include/gnu/types.h
obj/signals.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stddef.h
obj/signals.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stdarg.h
obj/signals.o: /usr/include/stdio_lim.h /usr/include/stdlib.h
obj/signals.o: /usr/include/sys/types.h /usr/include/time.h
obj/signals.o: /usr/include/endian.h /usr/include/bytesex.h
obj/signals.o: /usr/include/sys/select.h /usr/include/selectbits.h
obj/signals.o: /usr/include/alloca.h /usr/include/unistd.h
obj/signals.o: /usr/include/posix_opt.h /usr/include/confname.h
obj/signals.o: /usr/include/signal.h /usr/include/sigset.h
obj/signals.o: /usr/include/signum.h /usr/include/sigaction.h
obj/signals.o: /usr/include/sigcontext.h /usr/include/asm/sigcontext.h
obj/signals.o: /usr/include/sys/time.h /usr/include/timebits.h
obj/signals.o: include/global.h include/bug.h include/utils.h include/comm.h
obj/signals.o: include/whod.h include/signals.h
obj/sound.o: /usr/include/stdio.h /usr/include/libio.h
obj/sound.o: /usr/include/features.h /usr/include/sys/cdefs.h
obj/sound.o: /usr/include/gnu/stubs.h /usr/include/_G_config.h
obj/sound.o: /usr/include/gnu/types.h
obj/sound.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stddef.h
obj/sound.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stdarg.h
obj/sound.o: /usr/include/stdio_lim.h /usr/include/stdlib.h
obj/sound.o: /usr/include/sys/types.h /usr/include/time.h
obj/sound.o: /usr/include/endian.h /usr/include/bytesex.h
obj/sound.o: /usr/include/sys/select.h /usr/include/selectbits.h
obj/sound.o: /usr/include/alloca.h /usr/include/unistd.h
obj/sound.o: /usr/include/posix_opt.h /usr/include/confname.h
obj/sound.o: /usr/include/string.h include/global.h include/bug.h
obj/sound.o: include/comm.h include/interpreter.h include/handler.h
obj/sound.o: include/db.h include/spells.h include/trap.h include/utils.h
obj/sound.o: include/sound.h
obj/spell_parser.o: /usr/include/stdio.h /usr/include/libio.h
obj/spell_parser.o: /usr/include/features.h /usr/include/sys/cdefs.h
obj/spell_parser.o: /usr/include/gnu/stubs.h /usr/include/_G_config.h
obj/spell_parser.o: /usr/include/gnu/types.h
obj/spell_parser.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stddef.h
obj/spell_parser.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stdarg.h
obj/spell_parser.o: /usr/include/stdio_lim.h /usr/include/stdlib.h
obj/spell_parser.o: /usr/include/sys/types.h /usr/include/time.h
obj/spell_parser.o: /usr/include/endian.h /usr/include/bytesex.h
obj/spell_parser.o: /usr/include/sys/select.h /usr/include/selectbits.h
obj/spell_parser.o: /usr/include/alloca.h /usr/include/unistd.h
obj/spell_parser.o: /usr/include/posix_opt.h /usr/include/confname.h
obj/spell_parser.o: /usr/include/string.h /usr/include/assert.h
obj/spell_parser.o: include/global.h include/bug.h include/utils.h
obj/spell_parser.o: include/comm.h include/db.h include/interpreter.h
obj/spell_parser.o: include/handler.h include/spells.h include/constants.h
obj/spell_parser.o: include/act_off.h include/random.h include/multiclass.h
obj/spell_parser.o: include/fight.h include/act_info.h include/spell_parser.h
obj/spells.o: /usr/include/stdio.h /usr/include/libio.h
obj/spells.o: /usr/include/features.h /usr/include/sys/cdefs.h
obj/spells.o: /usr/include/gnu/stubs.h /usr/include/_G_config.h
obj/spells.o: /usr/include/gnu/types.h
obj/spells.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stddef.h
obj/spells.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stdarg.h
obj/spells.o: /usr/include/stdio_lim.h /usr/include/stdlib.h
obj/spells.o: /usr/include/sys/types.h /usr/include/time.h
obj/spells.o: /usr/include/endian.h /usr/include/bytesex.h
obj/spells.o: /usr/include/sys/select.h /usr/include/selectbits.h
obj/spells.o: /usr/include/alloca.h /usr/include/unistd.h
obj/spells.o: /usr/include/posix_opt.h /usr/include/confname.h
obj/spells.o: /usr/include/strings.h /usr/include/string.h include/global.h
obj/spells.o: include/bug.h include/utils.h include/comm.h include/db.h
obj/spells.o: include/interpreter.h include/handler.h include/constants.h
obj/spells.o: include/multiclass.h include/fight.h include/act_move.h
obj/spells.o: include/spells.h include/spell_parser.h
obj/trap.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
obj/trap.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
obj/trap.o: /usr/include/_G_config.h /usr/include/gnu/types.h
obj/trap.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stddef.h
obj/trap.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stdarg.h
obj/trap.o: /usr/include/stdio_lim.h /usr/include/stdlib.h
obj/trap.o: /usr/include/sys/types.h /usr/include/time.h
obj/trap.o: /usr/include/endian.h /usr/include/bytesex.h
obj/trap.o: /usr/include/sys/select.h /usr/include/selectbits.h
obj/trap.o: /usr/include/alloca.h /usr/include/unistd.h
obj/trap.o: /usr/include/posix_opt.h /usr/include/confname.h include/global.h
obj/trap.o: include/bug.h include/utils.h include/spells.h include/comm.h
obj/trap.o: include/db.h include/opinion.h include/constants.h
obj/trap.o: include/fight.h include/reception.h include/spell_parser.h
obj/trap.o: include/multiclass.h include/handler.h include/act_info.h
obj/trap.o: include/trap.h
obj/utils.o: /usr/include/stdio.h /usr/include/libio.h
obj/utils.o: /usr/include/features.h /usr/include/sys/cdefs.h
obj/utils.o: /usr/include/gnu/stubs.h /usr/include/_G_config.h
obj/utils.o: /usr/include/gnu/types.h
obj/utils.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stddef.h
obj/utils.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stdarg.h
obj/utils.o: /usr/include/stdio_lim.h /usr/include/stdlib.h
obj/utils.o: /usr/include/sys/types.h /usr/include/time.h
obj/utils.o: /usr/include/endian.h /usr/include/bytesex.h
obj/utils.o: /usr/include/sys/select.h /usr/include/selectbits.h
obj/utils.o: /usr/include/alloca.h /usr/include/unistd.h
obj/utils.o: /usr/include/posix_opt.h /usr/include/confname.h
obj/utils.o: /usr/include/malloc.h /usr/include/string.h
obj/utils.o: /usr/include/assert.h include/global.h include/bug.h
obj/utils.o: include/spells.h include/constants.h include/db.h
obj/utils.o: include/opinion.h include/comm.h include/hash.h
obj/utils.o: include/multiclass.h include/handler.h include/fight.h
obj/utils.o: include/act_info.h include/reception.h include/act_off.h
obj/utils.o: include/magic_utils.h include/limits.h include/act_skills.h
obj/utils.o: include/utils.h
obj/weather.o: /usr/include/stdio.h /usr/include/libio.h
obj/weather.o: /usr/include/features.h /usr/include/sys/cdefs.h
obj/weather.o: /usr/include/gnu/stubs.h /usr/include/_G_config.h
obj/weather.o: /usr/include/gnu/types.h
obj/weather.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stddef.h
obj/weather.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stdarg.h
obj/weather.o: /usr/include/stdio_lim.h /usr/include/stdlib.h
obj/weather.o: /usr/include/sys/types.h /usr/include/time.h
obj/weather.o: /usr/include/endian.h /usr/include/bytesex.h
obj/weather.o: /usr/include/sys/select.h /usr/include/selectbits.h
obj/weather.o: /usr/include/alloca.h /usr/include/unistd.h
obj/weather.o: /usr/include/posix_opt.h /usr/include/confname.h
obj/weather.o: /usr/include/string.h include/global.h include/bug.h
obj/weather.o: include/utils.h include/comm.h include/handler.h
obj/weather.o: include/interpreter.h include/db.h include/weather.h
obj/whod.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
obj/whod.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
obj/whod.o: /usr/include/_G_config.h /usr/include/gnu/types.h
obj/whod.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stddef.h
obj/whod.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stdarg.h
obj/whod.o: /usr/include/stdio_lim.h /usr/include/stdlib.h
obj/whod.o: /usr/include/sys/types.h /usr/include/time.h
obj/whod.o: /usr/include/endian.h /usr/include/bytesex.h
obj/whod.o: /usr/include/sys/select.h /usr/include/selectbits.h
obj/whod.o: /usr/include/alloca.h /usr/include/unistd.h
obj/whod.o: /usr/include/posix_opt.h /usr/include/confname.h
obj/whod.o: /usr/include/sys/socket.h /usr/include/socketbits.h
obj/whod.o: /usr/include/sockaddrcom.h /usr/include/asm/socket.h
obj/whod.o: /usr/include/asm/sockios.h /usr/include/sys/un.h
obj/whod.o: /usr/include/string.h /usr/include/netdb.h
obj/whod.o: /usr/include/rpc/netdb.h /usr/include/netinet/in.h
obj/whod.o: /usr/include/sys/time.h /usr/include/timebits.h
obj/whod.o: /usr/include/signal.h /usr/include/sigset.h /usr/include/signum.h
obj/whod.o: /usr/include/sigaction.h /usr/include/sigcontext.h
obj/whod.o: /usr/include/asm/sigcontext.h /usr/include/ctype.h
obj/whod.o: include/version.h include/global.h include/bug.h include/db.h
obj/whod.o: include/comm.h include/utils.h include/interpreter.h
obj/whod.o: include/multiclass.h include/whod.h
obj/events.o: /usr/include/stdio.h /usr/include/libio.h
obj/events.o: /usr/include/features.h /usr/include/sys/cdefs.h
obj/events.o: /usr/include/gnu/stubs.h /usr/include/_G_config.h
obj/events.o: /usr/include/gnu/types.h
obj/events.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stddef.h
obj/events.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stdarg.h
obj/events.o: /usr/include/stdio_lim.h /usr/include/stdlib.h
obj/events.o: /usr/include/sys/types.h /usr/include/time.h
obj/events.o: /usr/include/endian.h /usr/include/bytesex.h
obj/events.o: /usr/include/sys/select.h /usr/include/selectbits.h
obj/events.o: /usr/include/alloca.h /usr/include/unistd.h
obj/events.o: /usr/include/posix_opt.h /usr/include/confname.h
obj/events.o: /usr/include/string.h /usr/include/ctype.h include/global.h
obj/events.o: include/bug.h include/utils.h include/comm.h
obj/events.o: include/interpreter.h include/handler.h include/db.h
obj/events.o: include/spells.h include/limits.h include/constants.h
obj/events.o: include/spell_parser.h include/fight.h include/spec_procs.h
obj/events.o: include/opinion.h include/hash.h include/events.h
obj/specials/mob_actions.o: /usr/include/stdio.h /usr/include/libio.h
obj/specials/mob_actions.o: /usr/include/features.h /usr/include/sys/cdefs.h
obj/specials/mob_actions.o: /usr/include/gnu/stubs.h /usr/include/_G_config.h
obj/specials/mob_actions.o: /usr/include/gnu/types.h
obj/specials/mob_actions.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stddef.h
obj/specials/mob_actions.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stdarg.h
obj/specials/mob_actions.o: /usr/include/stdio_lim.h /usr/include/stdlib.h
obj/specials/mob_actions.o: /usr/include/sys/types.h /usr/include/time.h
obj/specials/mob_actions.o: /usr/include/endian.h /usr/include/bytesex.h
obj/specials/mob_actions.o: /usr/include/sys/select.h
obj/specials/mob_actions.o: /usr/include/selectbits.h /usr/include/alloca.h
obj/specials/mob_actions.o: /usr/include/unistd.h /usr/include/posix_opt.h
obj/specials/mob_actions.o: /usr/include/confname.h include/global.h
obj/specials/mob_actions.o: include/bug.h include/utils.h include/act_obj.h
obj/specials/mob_actions.o: include/act_off.h include/comm.h
obj/specials/mob_actions.o: include/constants.h include/db.h include/fight.h
obj/specials/mob_actions.o: include/handler.h include/hash.h include/limits.h
obj/specials/mob_actions.o: include/multiclass.h include/opinion.h
obj/specials/mob_actions.o: include/spell_parser.h include/trap.h
obj/specials/mob_actions.o: include/act_skills.h include/spec_procs.h
obj/specials/mob_actions.o: include/tracking.h include/mob_actions.h
obj/specials/spec_procs.o: /usr/include/stdio.h /usr/include/libio.h
obj/specials/spec_procs.o: /usr/include/features.h /usr/include/sys/cdefs.h
obj/specials/spec_procs.o: /usr/include/gnu/stubs.h /usr/include/_G_config.h
obj/specials/spec_procs.o: /usr/include/gnu/types.h
obj/specials/spec_procs.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stddef.h
obj/specials/spec_procs.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stdarg.h
obj/specials/spec_procs.o: /usr/include/stdio_lim.h /usr/include/stdlib.h
obj/specials/spec_procs.o: /usr/include/sys/types.h /usr/include/time.h
obj/specials/spec_procs.o: /usr/include/endian.h /usr/include/bytesex.h
obj/specials/spec_procs.o: /usr/include/sys/select.h
obj/specials/spec_procs.o: /usr/include/selectbits.h /usr/include/alloca.h
obj/specials/spec_procs.o: /usr/include/unistd.h /usr/include/posix_opt.h
obj/specials/spec_procs.o: /usr/include/confname.h /usr/include/string.h
obj/specials/spec_procs.o: /usr/include/signal.h /usr/include/sigset.h
obj/specials/spec_procs.o: /usr/include/signum.h /usr/include/sigaction.h
obj/specials/spec_procs.o: /usr/include/sigcontext.h
obj/specials/spec_procs.o: /usr/include/asm/sigcontext.h /usr/include/ctype.h
obj/specials/spec_procs.o: include/global.h include/bug.h include/utils.h
obj/specials/spec_procs.o: include/act_comm.h include/act_info.h
obj/specials/spec_procs.o: include/act_move.h include/act_obj.h
obj/specials/spec_procs.o: include/act_off.h include/act_social.h
obj/specials/spec_procs.o: include/act_wiz.h include/board.h include/comm.h
obj/specials/spec_procs.o: include/constants.h include/db.h include/fight.h
obj/specials/spec_procs.o: include/handler.h include/hash.h
obj/specials/spec_procs.o: include/interpreter.h include/limits.h
obj/specials/spec_procs.o: include/modify.h include/multiclass.h
obj/specials/spec_procs.o: include/opinion.h include/reception.h
obj/specials/spec_procs.o: include/shop.h include/spell_parser.h
obj/specials/spec_procs.o: include/spells.h include/breath_weapons.h
obj/specials/spec_procs.o: include/mob_actions.h include/tracking.h
obj/specials/spec_procs.o: include/spec_procs.h
obj/specials/breath_weapons.o: /usr/include/stdio.h /usr/include/libio.h
obj/specials/breath_weapons.o: /usr/include/features.h
obj/specials/breath_weapons.o: /usr/include/sys/cdefs.h
obj/specials/breath_weapons.o: /usr/include/gnu/stubs.h
obj/specials/breath_weapons.o: /usr/include/_G_config.h
obj/specials/breath_weapons.o: /usr/include/gnu/types.h
obj/specials/breath_weapons.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stddef.h
obj/specials/breath_weapons.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stdarg.h
obj/specials/breath_weapons.o: /usr/include/stdio_lim.h /usr/include/stdlib.h
obj/specials/breath_weapons.o: /usr/include/sys/types.h /usr/include/time.h
obj/specials/breath_weapons.o: /usr/include/endian.h /usr/include/bytesex.h
obj/specials/breath_weapons.o: /usr/include/sys/select.h
obj/specials/breath_weapons.o: /usr/include/selectbits.h
obj/specials/breath_weapons.o: /usr/include/alloca.h /usr/include/unistd.h
obj/specials/breath_weapons.o: /usr/include/posix_opt.h
obj/specials/breath_weapons.o: /usr/include/confname.h /usr/include/string.h
obj/specials/breath_weapons.o: /usr/include/signal.h /usr/include/sigset.h
obj/specials/breath_weapons.o: /usr/include/signum.h /usr/include/sigaction.h
obj/specials/breath_weapons.o: /usr/include/sigcontext.h
obj/specials/breath_weapons.o: /usr/include/asm/sigcontext.h
obj/specials/breath_weapons.o: /usr/include/ctype.h include/global.h
obj/specials/breath_weapons.o: include/bug.h include/utils.h
obj/specials/breath_weapons.o: include/act_off.h include/comm.h include/db.h
obj/specials/breath_weapons.o: include/limits.h include/multiclass.h
obj/specials/breath_weapons.o: include/spells.h include/breath_weapons.h
obj/specials/tracking.o: /usr/include/stdio.h /usr/include/libio.h
obj/specials/tracking.o: /usr/include/features.h /usr/include/sys/cdefs.h
obj/specials/tracking.o: /usr/include/gnu/stubs.h /usr/include/_G_config.h
obj/specials/tracking.o: /usr/include/gnu/types.h
obj/specials/tracking.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stddef.h
obj/specials/tracking.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stdarg.h
obj/specials/tracking.o: /usr/include/stdio_lim.h /usr/include/stdlib.h
obj/specials/tracking.o: /usr/include/sys/types.h /usr/include/time.h
obj/specials/tracking.o: /usr/include/endian.h /usr/include/bytesex.h
obj/specials/tracking.o: /usr/include/sys/select.h /usr/include/selectbits.h
obj/specials/tracking.o: /usr/include/alloca.h /usr/include/unistd.h
obj/specials/tracking.o: /usr/include/posix_opt.h /usr/include/confname.h
obj/specials/tracking.o: /usr/include/string.h include/global.h include/bug.h
obj/specials/tracking.o: include/utils.h include/act_move.h include/comm.h
obj/specials/tracking.o: include/constants.h include/db.h include/fight.h
obj/specials/tracking.o: include/handler.h include/hash.h
obj/specials/tracking.o: include/interpreter.h include/multiclass.h
obj/specials/tracking.o: include/opinion.h include/spells.h
obj/specials/tracking.o: include/tracking.h
obj/utils/sign.o: /usr/include/errno.h /usr/include/features.h
obj/utils/sign.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
obj/utils/sign.o: /usr/include/errnos.h /usr/include/linux/errno.h
obj/utils/sign.o: /usr/include/asm/errno.h /usr/include/stdio.h
obj/utils/sign.o: /usr/include/libio.h /usr/include/_G_config.h
obj/utils/sign.o: /usr/include/gnu/types.h
obj/utils/sign.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stddef.h
obj/utils/sign.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stdarg.h
obj/utils/sign.o: /usr/include/stdio_lim.h /usr/include/stdlib.h
obj/utils/sign.o: /usr/include/sys/types.h /usr/include/time.h
obj/utils/sign.o: /usr/include/endian.h /usr/include/bytesex.h
obj/utils/sign.o: /usr/include/sys/select.h /usr/include/selectbits.h
obj/utils/sign.o: /usr/include/alloca.h /usr/include/unistd.h
obj/utils/sign.o: /usr/include/posix_opt.h /usr/include/confname.h
obj/utils/sign.o: /usr/include/sys/socket.h /usr/include/socketbits.h
obj/utils/sign.o: /usr/include/sockaddrcom.h /usr/include/asm/socket.h
obj/utils/sign.o: /usr/include/asm/sockios.h /usr/include/sys/wait.h
obj/utils/sign.o: /usr/include/waitflags.h /usr/include/waitstatus.h
obj/utils/sign.o: /usr/include/netinet/in.h /usr/include/netdb.h
obj/utils/sign.o: /usr/include/rpc/netdb.h /usr/include/sys/time.h
obj/utils/sign.o: /usr/include/timebits.h /usr/include/fcntl.h
obj/utils/sign.o: /usr/include/fcntlbits.h /usr/include/ctype.h
obj/utils/sign.o: /usr/include/string.h include/global.h include/bug.h
obj/utils/mkworld.o: /usr/include/stdio.h /usr/include/libio.h
obj/utils/mkworld.o: /usr/include/features.h /usr/include/sys/cdefs.h
obj/utils/mkworld.o: /usr/include/gnu/stubs.h /usr/include/_G_config.h
obj/utils/mkworld.o: /usr/include/gnu/types.h
obj/utils/mkworld.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stddef.h
obj/utils/mkworld.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include/stdarg.h
obj/utils/mkworld.o: /usr/include/stdio_lim.h /usr/include/stdlib.h
obj/utils/mkworld.o: /usr/include/sys/types.h /usr/include/time.h
obj/utils/mkworld.o: /usr/include/endian.h /usr/include/bytesex.h
obj/utils/mkworld.o: /usr/include/sys/select.h /usr/include/selectbits.h
obj/utils/mkworld.o: /usr/include/alloca.h /usr/include/unistd.h
obj/utils/mkworld.o: /usr/include/posix_opt.h /usr/include/confname.h
obj/utils/mkworld.o: /usr/include/string.h /usr/include/ctype.h
obj/utils/mkworld.o: /usr/include/sys/timeb.h

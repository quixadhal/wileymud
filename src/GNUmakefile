# Must be GNU Make!

DATESTAMP=$(shell /bin/date "+%y.%m.%d")
OLDVERSION=$(shell cat .version.count)
VERSION=$(shell echo $(OLDVERSION) + 1 | bc)
DEPLINE=$(shell /bin/grep -n -e '^\#--- Comment for make deps ---' GNUmakefile | awk -F: '{print $$1}')

CC          = gcc #g++
MACHINE     = #-mcpu=pentium2 -march=pentium2

NEED_CRYPT  = -lcrypt #Linux needs this
#NEED_COMPAT = -lcompat #BSD needs this

W_FORMAT    = ##-Wformat -Wformat-security #-Wmissing-format-attribute
W_NITPICK   = ##-Wpointer-arith -Winline
W_MESSY     = ##-Wmissing-braces -Wparentheses -Wshadow #-Wredundant-decls
W_FUNC      = ###-Wstrict-prototypes #-Wmissing-prototypes
W_TYPE      = ##-Wcast-qual -Wcast-align -Wchar-subscripts -Wreturn-type -Wswitch #-Wwrite-strings
W_EXTRA     = ##-Wunused #-Wunreachable-code
W_C_ONLY    = ##-Wmissing-declarations -Wimplicit-int -Wimplicit-function-declaration #-Wsequence-point -Wnonnull
W_ANSI      = ###-pedantic
W_UBER      = ##-Wall #-W
W_ERROR     = ##-Werror

W_ALL       = $(W_ANSI) $(W_UBER) $(W_FORMAT) $(W_MESSY) $(W_FUNC) $(W_TYPE) $(W_EXTRA) $(W_NITPICK) $(W_ERROR)
W_CLANG     = $(W_C_ONLY)

W_FLAGS     = $(W_ALL) $(W_CLANG)

OPT_FLAG    = -pipe -O3 #-Wuninitialized

DEBUG_FLAG  = -ggdb
EFENCE      = #-lpthread -lefence
PROF_FLAG   = #-pg
DEPS_FLAG   = -MMD

SQL_INC     = -DUSE_PGSQL #-I/usr/include/postgresql
SQL_LIB     = -L/usr/lib/postgresql -lpq

CFLAGS      = $(MACHINE) $(W_FLAGS) $(DEBUG_FLAG) $(OPT_FLAG) $(PROF_FLAG) $(DEPS_FLAG) -Iinclude $(SQL_INC)
LDFLAGS     = $(DEBUG_FLAG) $(OPT_FLAG) $(PROF_FLAG)

SRC =	act_comm.c act_info.c act_move.c act_obj.c act_off.c \
	act_other.c act_skills.c act_social.c act_wiz.c \
	board.c bug.c comm.c constants.c db.c fight.c handler.c \
	hash.c interpreter.c limits.c magic.c magic_utils.c \
	modify.c multiclass.c opinion.c random.c reception.c \
	shop.c signals.c sound.c spell_parser.c \
	spells.c trap.c utils.c weather.c whod.c events.c sql.c
OBJ =	$(addprefix obj/, $(SRC:.c=.o))
TMP =	mob_actions.c spec_procs.c breath_weapons.c tracking.c
SPOB =	$(addprefix obj/, $(TMP:.c=.o))
SPEC =	$(addprefix specials/, $(TMP))
TMP2 =	sign.c
OBJ2 =	$(addprefix obj/, $(TMP2:.c=.o))
SRC2 =	$(addprefix aux/, $(TMP2))
TMP3 =	mkworld.c
OBJ3 =	$(addprefix obj/, $(TMP3:.c=.o))
SRC3 =	$(addprefix aux/, $(TMP3))
TMP4 =	genpasswd.c
OBJ4 =	$(addprefix obj/, $(TMP4:.c=.o))
SRC4 =	$(addprefix aux/, $(TMP4))
TMP5 =	bounce.c
OBJ5 =	$(addprefix obj/, $(TMP5:.c=.o))
SRC5 =	$(addprefix aux/, $(TMP5))
HDRS =	act_comm.h act_info.h act_move.h act_obj.h act_off.h \
	act_other.h act_skills.h act_social.h act_wiz.h \
	board.h bug.h comm.h constants.h db.h fight.h global.h \
	handler.h hash.h interpreter.h magic_utils.h \
	modify.h multiclass.h opinion.h \
	random.h reception.h shop.h sound.h spell_parser.h \
	spells.h trap.h utils.h weather.h whod.h signals.h events.h \
	mob_actions.h spec_procs.h breath_weapons.h tracking.h sql.h
HDR =	$(addprefix include/, $(HDRS))
VER =	$(addprefix include/, version.h)
IDENTDIR = libident-0.19
IDENTSRC = $(addprefix $(IDENTDIR)/, id_close.c id_open.c id_parse.c \
	id_query.c ident.c support.c version.c)
IDENTHDR = $(addprefix $(IDENTDIR)/, ident.h)
IDENTOBJ = $(IDENTSRC:.c=.o)
LIBS =	$(IDENTDIR)/libident.a
LIB =	-L$(IDENTDIR) -lident $(NEED_CRYPT) $(NEED_COMPAT) $(EFENCE) $(SQL_LIB)
AUX =	GNUmakefile .indent.pro HACKLOG version.proto .version.count

ALLSRC = $(SRC) $(SPEC) $(SRC2) $(SRC3) $(SRC4) $(SRC5) $(IDENTSRC)
ALLOBJ = $(OBJ) $(SPOB) $(OBJ2) $(OBJ3) $(OBJ4) $(OBJ5) $(IDENTOBJ)
ALLHDR = $(HDR)
ALLDEP = $(ALLOBJ:.o=.d)

BINDIR = bin
BININSTDIR = ../bin
ETCDIR = etc
ETCINSTDIR = ../etc
INITDIR = etc/init.d
INITINSTDIR = ../etc

BIN = wileyloop wileymud sign mkworld genpasswd bounce
ETC = down.time
INIT = wiley

ALLBIN = $(addprefix $(BINDIR)/, $(BIN))
ALLETC = $(addprefix $(ETCDIR)/, $(ETC))
ALLINIT = $(addprefix $(INITDIR)/, $(INIT))

ALLBININST = $(addprefix $(BININSTDIR)/, $(BIN))
ALLETCINST = $(addprefix $(ETCINSTDIR)/, $(ETC))
ALLINITINST = $(addprefix $(INITINSTDIR)/, $(INIT))
INSTDIRS = $(BININSTDIR) $(ETCINSTDIR) $(INITINSTDIR)

all : $(ALLBIN)

.PHONY : .installing

.installing :
	@echo 'Installing WileyMUD binaries!'

$(ALLBININST) : .installing $(ALLBIN)
	install -D -b -g divine -m 0750 $(addprefix $(BINDIR)/, $(notdir $@)) $(addprefix $(BININSTDIR)/, $(notdir $@))

$(ALLETCINST) : .installing $(ALLETC)
	install -D -b -g divine -m 0640 $(addprefix $(ETCDIR)/, $(notdir $@)) $(addprefix $(ETCINSTDIR)/, $(notdir $@))

$(ALLINITINST) : .installing $(ALLINIT)
	install -D -b -g divine -m 0750 $(addprefix $(INITDIR)/, $(notdir $@)) $(addprefix $(INITINSTDIR)/, $(notdir $@))

$(INSTDIRS) : .installing
	mkdir -p -m 750 $@

install : $(INSTDIRS) $(ALLBININST) $(ALLETCINST) $(ALLINITINST)

#.deps/%.d: %.c
#	set -e; $(CC) -M $(CFLAGS) $< \
#		| sed 's!\($*\)\.o[ :]*!\1.o $@ : !g' > $@; \
#		[ -s $@ ] || rm -f $@
#
#include $(addprefix .deps/, $(ALLSRC:.c=.d))

$(VER) : version.proto .version.count
	@echo '/* Auto-generated from version.proto -- DO NOT EDIT */' > $@
	@echo 's/XxX/$(VERSION)/' >.sed.cmds
	@echo 's/YyY/$(DATESTAMP)/' >>.sed.cmds
	@sed -f .sed.cmds <version.proto >>$@
	@rm -f .sed.cmds
	@echo New version file generated.

$(BINDIR)/wileyloop : wileyloop.sh
	@cp -p $< $@

$(BINDIR)/wileymud : $(OBJ) $(SPOB) $(LIBS) $(VER)
	@echo Heave Ho!  Load the beastie!
	$(CC) $(LDFLAGS) -o $@ $(OBJ) $(SPOB) $(LIB)
	@echo $(VERSION) >.version.count
	@ls -l $@

$(BINDIR)/sign : $(OBJ2)
	@echo Make the downtime notice program...
	$(CC) $(LDFLAGS) -o $@ $(OBJ2) $(LIB)
	@ls -l $@

$(BINDIR)/mkworld : $(OBJ3)
	@echo Make the world editing program...
	$(CC) $(LDFLAGS) -o $@ $(OBJ3) $(LIB)
	@ls -l $@

$(BINDIR)/genpasswd : $(OBJ4)
	@echo Make the password generation program...
	$(CC) $(LDFLAGS) -o $@ $(OBJ4) $(LIB)
	@ls -l $@

$(BINDIR)/bounce : $(OBJ5)
	@echo Make the port bouncer program...
	$(CC) $(LDFLAGS) -o $@ $(OBJ5) $(LIB)
	@ls -l $@

$(OBJ): obj/%o : %c
	$(CC) $(CFLAGS) -c $< -o $@

$(SPOB): obj/%o : specials/%c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ2): obj/%o : aux/%c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ3): obj/%o : aux/%c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ4): obj/%o : aux/%c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ5): obj/%o : aux/%c
	$(CC) $(CFLAGS) -c $< -o $@

$(IDENTDIR)/libident.a: $(IDENTSRC) $(IDENTHDR)
	(cd libident-0.19;make linux)

.deps : $(OBJ) $(SPOB) $(OBJ2) $(OBJ3) $(OBJ4) $(OBJ5)
	@echo 'Building dependancies'
	@cat obj/*.d >.deps

.PHONY : deps

deps : .deps GNUmakefile
	@echo 'Self-Modifying GNUmakefile'
	@head -$(DEPLINE) GNUmakefile >.new_makefile
	@cat .deps >>.new_makefile
	@mv -f GNUmakefile .old_makefile
	@mv -f .new_makefile GNUmakefile

.PHONY : clean

clean :
	@echo -n Wiping away intermediate clutter...
	@rm -f $(ALLOBJ) $(ALLDEP) *.bak *~
	@find . -name \*.d -a -type f | xargs rm -f
	@echo done.

spotless : clean
	@echo -n Destroying the world and all those within it...
	@rm -f $(ALLBIN)
	@rm -f tags refs
	@rm -f $(LIBS)
	@echo done!

tags : $(SRC) $(SPEC) $(VER) $(HDR)
	@echo -n Making tag files...
	@ctags $(SRC) $(SPEC) $(VER) $(HDR)
	@echo done.

#--- Comment for make deps ---
obj/act_comm.o: act_comm.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/interpreter.h include/handler.h include/db.h \
  include/spells.h include/constants.h include/mudlimits.h \
  include/multiclass.h include/act_comm.h
obj/act_info.o: act_info.c include/global.h include/bug.h \
  include/version.h include/utils.h include/comm.h include/interpreter.h \
  include/handler.h include/db.h include/spells.h include/mudlimits.h \
  include/trap.h include/hash.h include/constants.h \
  include/spell_parser.h include/whod.h include/multiclass.h \
  include/modify.h include/act_wiz.h include/act_skills.h \
  include/spec_procs.h include/tracking.h include/act_info.h
obj/act_move.o: act_move.c include/global.h include/utils.h include/bug.h \
  include/comm.h include/interpreter.h include/handler.h include/db.h \
  include/spells.h include/trap.h include/constants.h include/whod.h \
  include/multiclass.h include/fight.h include/reception.h \
  include/magic_utils.h include/spell_parser.h include/act_info.h \
  include/act_skills.h include/act_move.h
obj/act_obj.o: act_obj.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/interpreter.h include/handler.h include/db.h \
  include/spells.h include/trap.h include/constants.h \
  include/spell_parser.h include/multiclass.h include/mudlimits.h \
  include/fight.h include/act_info.h include/act_obj.h
obj/act_off.o: act_off.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/interpreter.h include/handler.h include/db.h \
  include/spells.h include/mudlimits.h include/opinion.h \
  include/multiclass.h include/constants.h include/spec_procs.h \
  include/fight.h include/act_skills.h include/act_move.h \
  include/spell_parser.h include/act_info.h include/breath_weapons.h \
  include/act_off.h
obj/act_other.o: act_other.c include/global.h include/bug.h \
  include/utils.h include/comm.h include/interpreter.h include/handler.h \
  include/db.h include/spells.h include/mudlimits.h include/constants.h \
  include/spell_parser.h include/reception.h include/multiclass.h \
  include/fight.h include/spec_procs.h include/act_skills.h \
  include/act_comm.h include/modify.h include/act_other.h
obj/act_skills.o: act_skills.c include/global.h include/bug.h \
  include/utils.h include/comm.h include/constants.h include/db.h \
  include/fight.h include/handler.h include/interpreter.h \
  include/multiclass.h include/spell_parser.h include/spells.h \
  include/spec_procs.h include/act_info.h include/act_skills.h
obj/act_social.o: act_social.c include/global.h include/bug.h \
  include/utils.h include/comm.h include/interpreter.h include/handler.h \
  include/db.h include/spells.h include/multiclass.h include/act_social.h
obj/act_wiz.o: act_wiz.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/interpreter.h include/handler.h include/db.h \
  include/spells.h include/mudlimits.h include/constants.h \
  include/spell_parser.h include/board.h include/whod.h \
  include/reception.h include/spec_procs.h include/multiclass.h \
  include/act_skills.h include/act_info.h include/fight.h include/hash.h \
  include/weather.h include/modify.h include/act_wiz.h
obj/board.o: board.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/db.h include/interpreter.h include/multiclass.h \
  include/modify.h include/handler.h include/act_info.h include/board.h
obj/bounce.o: aux/bounce.c
obj/breath_weapons.o: specials/breath_weapons.c include/global.h \
  include/bug.h include/utils.h include/act_off.h include/comm.h \
  include/db.h include/mudlimits.h include/multiclass.h include/spells.h \
  include/breath_weapons.h
obj/bug.o: bug.c include/global.h include/utils.h include/comm.h \
  include/multiclass.h include/bug.h
obj/comm.o: comm.c include/global.h include/bug.h include/utils.h \
  include/interpreter.h include/handler.h include/db.h include/modify.h \
  include/whod.h include/multiclass.h include/weather.h \
  include/mudlimits.h include/spell_parser.h include/sound.h \
  include/fight.h include/mob_actions.h include/act_other.h \
  include/comm.h
obj/constants.o: constants.c include/global.h include/bug.h \
  include/mudlimits.h include/trap.h include/constants.h
obj/db.o: db.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/handler.h include/mudlimits.h include/opinion.h \
  include/hash.h include/constants.h include/spells.h \
  include/spell_parser.h include/reception.h include/weather.h \
  include/modify.h include/fight.h include/act_social.h \
  include/spec_procs.h include/multiclass.h include/board.h \
  include/interpreter.h include/db.h
obj/events.o: events.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/interpreter.h include/handler.h include/db.h \
  include/spells.h include/mudlimits.h include/constants.h \
  include/spell_parser.h include/fight.h include/spec_procs.h \
  include/opinion.h include/hash.h include/events.h
obj/fight.o: fight.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/handler.h include/interpreter.h include/db.h \
  include/constants.h include/spells.h include/spell_parser.h \
  include/mudlimits.h include/random.h include/act_move.h \
  include/reception.h include/multiclass.h include/act_wiz.h \
  include/act_skills.h include/opinion.h include/spec_procs.h \
  include/mob_actions.h include/act_off.h include/fight.h
obj/genpasswd.o: aux/genpasswd.c
obj/handler.o: handler.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/db.h include/interpreter.h include/spells.h \
  include/spell_parser.h include/constants.h include/fight.h \
  include/modify.h include/multiclass.h include/opinion.h \
  include/act_wiz.h include/handler.h
obj/hash.o: hash.c include/global.h include/bug.h include/comm.h \
  include/utils.h include/hash.h
obj/interpreter.o: interpreter.c include/global.h include/bug.h \
  include/comm.h include/version.h include/db.h include/utils.h \
  include/mudlimits.h include/constants.h include/act_comm.h \
  include/act_info.h include/act_move.h include/act_obj.h \
  include/act_off.h include/act_other.h include/act_skills.h \
  include/act_social.h include/act_wiz.h include/spells.h \
  include/spell_parser.h include/modify.h include/whod.h include/events.h \
  include/random.h include/board.h include/multiclass.h include/handler.h \
  include/reception.h include/tracking.h include/interpreter.h
obj/limits.o: limits.c include/global.h include/bug.h include/spells.h \
  include/comm.h include/db.h include/spell_parser.h include/constants.h \
  include/utils.h include/multiclass.h include/fight.h \
  include/reception.h include/interpreter.h include/handler.h \
  include/act_obj.h include/act_other.h include/mudlimits.h
obj/magic.o: magic.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/handler.h include/mudlimits.h include/db.h \
  include/constants.h include/spells.h include/spell_parser.h \
  include/multiclass.h include/fight.h include/opinion.h \
  include/reception.h include/magic_utils.h include/act_off.h \
  include/act_obj.h include/act_info.h
obj/magic_utils.o: magic_utils.c include/global.h include/bug.h \
  include/utils.h include/comm.h include/db.h include/spells.h \
  include/handler.h include/mudlimits.h include/fight.h \
  include/spell_parser.h include/multiclass.h include/magic_utils.h
obj/mkworld.o: aux/mkworld.c
obj/mob_actions.o: specials/mob_actions.c include/global.h include/bug.h \
  include/utils.h include/act_obj.h include/act_off.h include/comm.h \
  include/constants.h include/db.h include/fight.h include/handler.h \
  include/hash.h include/mudlimits.h include/multiclass.h \
  include/opinion.h include/spell_parser.h include/trap.h \
  include/act_skills.h include/spec_procs.h include/tracking.h \
  include/mob_actions.h
obj/modify.o: modify.c include/global.h include/bug.h include/utils.h \
  include/interpreter.h include/handler.h include/db.h include/comm.h \
  include/multiclass.h include/modify.h
obj/multiclass.o: multiclass.c include/global.h include/bug.h \
  include/utils.h include/comm.h include/db.h include/interpreter.h \
  include/handler.h include/spells.h include/mudlimits.h \
  include/opinion.h include/constants.h include/multiclass.h
obj/opinion.o: opinion.c include/global.h include/bug.h include/utils.h \
  include/spells.h include/constants.h include/comm.h include/db.h \
  include/multiclass.h include/opinion.h
obj/random.o: random.c include/global.h include/utils.h include/comm.h \
  include/random.h
obj/reception.o: reception.c include/global.h include/bug.h \
  include/comm.h include/handler.h include/db.h include/interpreter.h \
  include/utils.h include/spells.h include/multiclass.h \
  include/act_social.h include/spec_procs.h include/reception.h
obj/shop.o: shop.c include/global.h include/bug.h include/comm.h \
  include/handler.h include/db.h include/interpreter.h include/utils.h \
  include/constants.h include/act_comm.h include/multiclass.h \
  include/act_social.h include/act_wiz.h include/spec_procs.h \
  include/shop.h
obj/sign.o: aux/sign.c include/global.h include/bug.h
obj/signals.o: signals.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/whod.h include/signals.h
obj/sound.o: sound.c include/global.h include/bug.h include/comm.h \
  include/interpreter.h include/handler.h include/db.h include/spells.h \
  include/trap.h include/utils.h include/sound.h
obj/spec_procs.o: specials/spec_procs.c include/global.h include/bug.h \
  include/utils.h include/act_comm.h include/act_info.h \
  include/act_move.h include/act_obj.h include/act_off.h \
  include/act_social.h include/act_wiz.h include/board.h include/comm.h \
  include/constants.h include/db.h include/fight.h include/handler.h \
  include/hash.h include/interpreter.h include/mudlimits.h \
  include/modify.h include/multiclass.h include/opinion.h \
  include/reception.h include/shop.h include/spell_parser.h \
  include/spells.h include/breath_weapons.h include/mob_actions.h \
  include/tracking.h include/spec_procs.h
obj/spell_parser.o: spell_parser.c include/global.h include/bug.h \
  include/utils.h include/comm.h include/db.h include/interpreter.h \
  include/handler.h include/spells.h include/constants.h \
  include/act_off.h include/random.h include/multiclass.h include/fight.h \
  include/act_info.h include/spell_parser.h
obj/spells.o: spells.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/db.h include/interpreter.h include/handler.h \
  include/constants.h include/multiclass.h include/fight.h \
  include/act_move.h include/spells.h include/spell_parser.h
obj/tracking.o: specials/tracking.c include/global.h include/bug.h \
  include/utils.h include/act_move.h include/comm.h include/constants.h \
  include/db.h include/fight.h include/handler.h include/hash.h \
  include/interpreter.h include/multiclass.h include/opinion.h \
  include/spells.h include/tracking.h
obj/trap.o: trap.c include/global.h include/bug.h include/utils.h \
  include/spells.h include/comm.h include/db.h include/opinion.h \
  include/constants.h include/fight.h include/reception.h \
  include/spell_parser.h include/multiclass.h include/handler.h \
  include/act_info.h include/trap.h
obj/utils.o: utils.c include/global.h include/bug.h include/spells.h \
  include/constants.h include/db.h include/opinion.h include/comm.h \
  include/hash.h include/multiclass.h include/handler.h include/fight.h \
  include/act_info.h include/reception.h include/act_off.h \
  include/magic_utils.h include/mudlimits.h include/act_skills.h \
  include/utils.h
obj/weather.o: weather.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/handler.h include/interpreter.h include/db.h \
  include/weather.h
obj/whod.o: whod.c include/version.h include/global.h include/bug.h \
  include/db.h include/comm.h include/utils.h include/interpreter.h \
  include/multiclass.h include/whod.h

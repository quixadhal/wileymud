# Must be GNU Make!

DATESTAMP	= $(shell /bin/date "+%Y-%m-%d")
DEPLINE		= $(shell /bin/grep -n -e '^\#--- Comment for make deps ---' GNUmakefile | awk -F: '{print $$1}')

CC		= clang #gcc #g++
MAKE		= make #gmake

PRODBIN		= ../bin
PRODOWN		= wiley.wiley
PRODMODE	= 0750

BINDIR		= bin
IDENTDIR	= libident-0.19

NEED_CRYPT	= -lcrypt #Linux needs this
#NEED_COMPAT	= -lcompat #BSD needs this
NEED_MATH	= -lm #math for tracking and such
NEED_SSL	= -lcrypto #MD5 checksum code
SQL_INC		= 
SQL_LIB		= -lpq
#SQL_LIB		= -lsqlite3
#SQL_INC		= -DUSE_PGSQL -I/usr/include/postgresql
#SQL_LIB		= -L/usr/lib/postgresql -lpq -lecpg
ID_DEP		= $(IDENTDIR)/libident.a
ID_INC		= #
ID_LIB		= -L$(IDENTDIR) -lident
PCRE_LIB	= -lpcre
IMC_INC		= #-DIMC -DIMCWILEY
I3_INC		= -DI3

HOSTTYPE	= $(shell arch)
ifeq "$(HOSTTYPE)" "x86_64"
M_ARCH		= -m64
else
M_ARCH		= -m32
endif

W_ERROR         = -Werror
W_ANSI          = #-pedantic
W_UBER          = -Wall
W_FORMAT        = -Wformat -Wformat-security -Wmissing-format-attribute
W_MESSY         = -Wmissing-braces -Wparentheses -Wshadow -Wredundant-decls
W_TYPE          = -Wcast-qual -Wcast-align -Wchar-subscripts -Wreturn-type -Wswitch -Wwrite-strings
W_EXTRA         = -Wunused -Wuninitialized #-Wunreachable-code
W_NITPICK       = -Wpointer-arith -Winline
W_CONLY		= -Wmissing-declarations -Wmissing-prototypes -Wstrict-prototypes 

#W_FLAGS		= $(W_ERROR) $(W_ANSI) $(W_UBER) $(W_FORMAT) $(W_MESSY) $(W_TYPE) $(W_EXTRA) $(W_NITPICK) $(W_CONLY)
W_FLAGS		= -Werror -Wall # -Weverything

#OPT_FLAG	= -pipe -O3 #-Wuninitialized
OPT_FLAG	= -O0 #-pipe -O3 #-Wuninitialized

#DEBUG_FLAG	= -ggdb3
DEBUG_FLAG	= -g
ADDL_DEBUG	= #-DTIME_DEBUG
EFENCE		= #-lpthread -lefence
PROF_FLAG	= #-pg
DYN_FLAG	= -rdynamic

INC		= -Iinclude $(ID_INC) $(IMC_INC) $(I3_INC) $(SQL_INC)
LIB		= $(ID_LIB) $(NEED_MATH) $(NEED_SSL) $(NEED_CRYPT) $(NEED_COMPAT) $(EFENCE) $(PCRE_LIB) $(SQL_LIB)
EXTRALIB	= $(NEED_MATH) $(NEED_CRYPT) $(NEED_COMPAT) $(EFENCE)

CFLAGS		= $(W_FLAGS) $(DEBUG_FLAG) $(OPT_FLAG) $(PROF_FLAG) $(M_ARCH) $(INC)
LDFLAGS		= $(DEBUG_FLAG) $(OPT_FLAG) $(PROF_FLAG) $(DYN_FLAG)

VER		= $(addprefix include/, version.h)
OLDV		= $(shell perl -ne '$$i = $$1 if /Version\s+\d\.(\d+)w3-gamma/i; END { $$i = 0 if !defined $$i; print "$$i\n";}' < $(VER))
VERSION		= $(shell echo $(OLDV) + 1 | bc)

tOBJ =	mob_actions.o spec_procs.o breath_weapons.o tracking.o act_comm.o \
	act_info.o act_move.o act_obj.o act_off.o act_other.o \
	act_skills.o act_social.o act_wiz.o board.o bug.o \
	comm.o global.o db.o fight.o handler.o \
	hash.o interpreter.o limits.o magic.o magic_utils.o \
	modify.o multiclass.o opinion.o random.o reception.o \
	shop.o signals.o sound.o spell_parser.o spells.o \
	trap.o utils.o weather.o whod.o events.o \
	ban.o sha256.o imc.o i3.o sql.o \
	stringmap.o scheduler.o crc32.o reboot.o rent.o \
	help.o base64.o 2fa.o
OBJ =	$(addprefix obj/, $(tOBJ))

tIDOBJ		= id_close.o id_open.o id_parse.o \
		  id_query.o ident.o support.o version.o
IDOBJ		= $(addprefix obj/, $(tIDOBJ))

all : version depend bin

bin : $(BINDIR)/sign $(BINDIR)/bounce $(BINDIR)/genpasswd $(BINDIR)/mkworld $(BINDIR)/wileymud 

testhost :
	@echo "$(HOSTTYPE)"
	@echo "$(M_ARCH)"

$(BINDIR)/wileymud : $(ID_DEP) $(OBJ)
	@echo Heave Ho!  Load the beastie!
	$(CC) $(LDFLAGS) -o $@ $(OBJ) $(LIB)
	@ls -l $(BINDIR)

$(BINDIR)/mkworld : obj/mkworld.o
	@echo Make the world editing program...
	$(CC) $(LDFLAGS) -o $@ $< $(EXTRALIB)

$(BINDIR)/sign : obj/sign.o
	@echo Make the downtime notice program...
	$(CC) $(LDFLAGS) -o $@ $< $(EXTRALIB)

$(BINDIR)/genpasswd : obj/genpasswd.o
	@echo Make the password generation program...
	$(CC) $(LDFLAGS) -o $@ $< $(EXTRALIB)

$(BINDIR)/bounce : obj/bounce.o
	@echo Make the port bouncer program...
	$(CC) $(LDFLAGS) -o $@ $< $(EXTRALIB)

$(ID_DEP): $(IDOBJ)
	(cd $(IDENTDIR);$(MAKE) linux)

.PHONY : depend

depend :
	@echo 'Building dependancies'
	@find . -name \*.c -a -type f -print0 | grep -z -Z -v convert | xargs -0 -P 1 -r $(CC) $(CFLAGS) -MM >.deps
	@echo 'Self-Modifying GNUmakefile'
	@head -$(DEPLINE) GNUmakefile >.new_makefile
	@cat .deps | perl -ne '{ s!^([^:]+):!obj/$$1:!; print $$_; }' >>.new_makefile
	@mv -f GNUmakefile .old_makefile
	@mv -f .new_makefile GNUmakefile
	@echo 'Now do "make bin"!'

.PHONY : version

version :
	@echo '/* Auto-generated -- DO NOT EDIT */' > $(VER)
	@echo '#ifndef _VERSION_H' >> $(VER)
	@echo '#define _VERSION_H' >> $(VER)
	@echo '' >> $(VER)
	@echo "#define VERSION_STR \\\\" >> $(VER)
	@echo "\"\\\\r\\\\n*** Welcome to WileyMUD III, Quixadhal's Version 2.$(VERSION)w3-gamma ($(DATESTAMP)) ***\\\\r\\\\n\"" >> $(VER)
	@echo "#define VERSION_BASE \"WileyMUD III\"" >> $(VER)
	@echo "#define VERSION_BUILD \"2.$(VERSION)g\"" >> $(VER)
	@echo "#define VERSION_DATE \"$(DATESTAMP)\"" >> $(VER)
	@echo '#endif /* _VERSION_H */' >> $(VER)
	@echo New version file generated.

.PHONY : spotless

spotless :
	@echo -n Wiping away all generated files...
	@find . -name \*.o -a -type f -print0 | xargs -0 -P 1 -r rm -f
	@find . -name \*.a -a -type f -print0 | xargs -0 -P 1 -r rm -f
	@rm -f tags
	@rm -f $(BINDIR)/sign $(BINDIR)/bounce $(BINDIR)/genpasswd $(BINDIR)/mkworld $(BINDIR)/wileymud 
	@echo done.

.PHONY : clean

clean :
	@echo -n Wiping away intermediate clutter...
	@find . -name \*.o -a -type f -print0 | xargs -0 -P 1 -r rm -f
	@find . -name \*.a -a -type f -print0 | xargs -0 -P 1 -r rm -f
	@rm -f tags
	@echo done.

.PHONY : tags

tags :
	@echo -n Making tag files...
	@find . -name \*.[ch] -a -type f -print0 | grep -z -Z -v convert | xargs -0 -P 1 -r ctags
	@echo done.

.PHONY : install

install : bin
	@echo -n Installing into production environment...
	@-mv $(PRODBIN)/wileymud $(PRODBIN)/wileymud.old
	@cp -vp $(BINDIR)/wileymud $(BINDIR)/sign $(BINDIR)/bounce $(BINDIR)/genpasswd $(BINDIR)/mkworld $(PRODBIN)/
	@-chown $(PRODOWN) $(PRODBIN)/wileymud $(PRODBIN)/sign $(PRODBIN)/bounce $(PRODBIN)/genpasswd $(PRODBIN)/mkworld
	@chmod $(PRODMODE) $(PRODBIN)/wileymud $(PRODBIN)/sign $(PRODBIN)/bounce $(PRODBIN)/genpasswd $(PRODBIN)/mkworld
	@cp -vp extras/wileyloop.sh $(PRODBIN)/wileyloop
	@-chown $(PRODOWN) $(PRODBIN)/wileyloop
	@chmod $(PRODMODE) $(PRODBIN)/wileyloop 
	@echo done.

%c : %pgc
	ecpg $(INC) -c $< -o $@

obj/%o : %c
	$(CC) $(CFLAGS) -c $< -o $@

obj/sign.o obj/mkworld.o obj/genpasswd.o obj/bounce.o : obj/%o : extras/%c
	$(CC) $(CFLAGS) -c $< -o $@

#--- Comment for make deps ---
obj/act_wiz.o: act_wiz.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/interpreter.h include/handler.h include/db.h \
  include/spells.h include/mudlimits.h include/spell_parser.h \
  include/board.h include/whod.h include/reception.h \
  include/spec_procs.h include/multiclass.h include/act_skills.h \
  include/act_info.h include/fight.h include/hash.h include/weather.h \
  include/modify.h include/tracking.h include/i3.h include/sql.h \
  include/reboot.h include/rent.h include/act_wiz.h
obj/spec_procs.o: spec_procs.c include/global.h include/bug.h include/utils.h \
  include/act_comm.h include/act_info.h include/act_move.h \
  include/act_obj.h include/act_off.h include/act_social.h \
  include/act_wiz.h include/board.h include/comm.h include/db.h \
  include/handler.h include/hash.h include/interpreter.h \
  include/mudlimits.h include/modify.h include/multiclass.h \
  include/opinion.h include/reception.h include/shop.h include/spells.h \
  include/spell_parser.h include/fight.h include/breath_weapons.h \
  include/mob_actions.h include/tracking.h include/weather.h \
  include/spec_procs.h
obj/ban.o: ban.c include/global.h include/utils.h include/comm.h \
  include/modify.h include/interpreter.h include/db.h include/bug.h \
  include/sql.h include/ban.h
obj/whod.o: whod.c include/version.h include/global.h include/bug.h \
  include/db.h include/comm.h include/utils.h include/interpreter.h \
  include/multiclass.h include/i3.h include/sha256.h include/modify.h \
  include/sql.h include/whod.h
obj/limits.o: limits.c include/global.h include/bug.h include/spells.h \
  include/comm.h include/db.h include/spell_parser.h include/utils.h \
  include/multiclass.h include/fight.h include/reception.h \
  include/interpreter.h include/handler.h include/act_obj.h \
  include/act_other.h include/weather.h include/mudlimits.h
obj/rent.o: rent.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/sql.h include/rent.h
obj/reception.o: reception.c include/global.h include/bug.h include/comm.h \
  include/handler.h include/db.h include/interpreter.h include/utils.h \
  include/spells.h include/multiclass.h include/act_social.h \
  include/spec_procs.h include/rent.h include/weather.h \
  include/reception.h
obj/magic_utils.o: magic_utils.c include/global.h include/bug.h \
  include/utils.h include/comm.h include/db.h include/spells.h \
  include/handler.h include/mudlimits.h include/fight.h \
  include/spell_parser.h include/multiclass.h include/magic_utils.h
obj/sound.o: sound.c include/global.h include/bug.h include/comm.h \
  include/interpreter.h include/handler.h include/db.h include/spells.h \
  include/trap.h include/utils.h include/sound.h
obj/signals.o: signals.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/whod.h include/signals.h
obj/global.o: global.c include/global.h include/bug.h include/mudlimits.h \
  include/trap.h
obj/weather.o: weather.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/handler.h include/interpreter.h include/db.h \
  include/sql.h include/weather.h
obj/breath_weapons.o: breath_weapons.c include/global.h include/bug.h \
  include/utils.h include/act_off.h include/comm.h include/db.h \
  include/mudlimits.h include/multiclass.h include/spells.h \
  include/breath_weapons.h
obj/comm.o: comm.c include/global.h include/sql.h include/i3.h include/bug.h \
  include/utils.h include/interpreter.h include/handler.h include/db.h \
  include/modify.h include/whod.h include/multiclass.h include/weather.h \
  include/mudlimits.h include/spells.h include/spell_parser.h \
  include/sound.h include/fight.h include/mob_actions.h \
  include/act_other.h include/signals.h include/ban.h include/board.h \
  include/reboot.h include/comm.h
obj/trap.o: trap.c include/global.h include/bug.h include/utils.h \
  include/spells.h include/comm.h include/db.h include/opinion.h \
  include/fight.h include/reception.h include/spell_parser.h \
  include/multiclass.h include/handler.h include/act_info.h \
  include/trap.h
obj/base64.o: base64.c include/base64.h
obj/multiclass.o: multiclass.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/db.h include/interpreter.h include/handler.h \
  include/spells.h include/mudlimits.h include/opinion.h \
  include/multiclass.h
obj/id_open.o: libident-0.19/id_open.c libident-0.19/ident.h
obj/version.o: libident-0.19/version.c
obj/lookup-tester.o: libident-0.19/lookup-tester.c libident-0.19/ident.h
obj/id_parse.o: libident-0.19/id_parse.c libident-0.19/ident.h
obj/id_query.o: libident-0.19/id_query.c libident-0.19/ident.h
obj/ident.o: libident-0.19/ident.c libident-0.19/ident.h
obj/ident-tester.o: libident-0.19/ident-tester.c libident-0.19/ident.h
obj/id_close.o: libident-0.19/id_close.c libident-0.19/ident.h
obj/support.o: libident-0.19/support.c libident-0.19/ident.h
obj/utils.o: utils.c include/global.h include/bug.h include/spells.h \
  include/db.h include/opinion.h include/comm.h include/hash.h \
  include/multiclass.h include/handler.h include/fight.h \
  include/act_info.h include/reception.h include/act_off.h \
  include/magic_utils.h include/mudlimits.h include/act_skills.h \
  include/i3.h include/utils.h
obj/db.o: db.c include/global.h include/i3.h include/bug.h include/utils.h \
  include/comm.h include/handler.h include/mudlimits.h include/opinion.h \
  include/hash.h include/spells.h include/spell_parser.h \
  include/reception.h include/weather.h include/modify.h include/fight.h \
  include/act_social.h include/spec_procs.h include/multiclass.h \
  include/board.h include/interpreter.h include/ban.h include/sql.h \
  include/reboot.h include/rent.h include/stringmap.h include/help.h \
  include/db.h
obj/act_social.o: act_social.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/interpreter.h include/handler.h include/db.h \
  include/spells.h include/multiclass.h include/act_social.h
obj/act_move.o: act_move.c include/global.h include/utils.h include/bug.h \
  include/comm.h include/interpreter.h include/handler.h include/db.h \
  include/spells.h include/trap.h include/whod.h include/multiclass.h \
  include/fight.h include/reception.h include/magic_utils.h \
  include/spell_parser.h include/act_info.h include/act_skills.h \
  include/act_move.h
obj/handler.o: handler.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/db.h include/interpreter.h include/spells.h \
  include/spell_parser.h include/fight.h include/modify.h \
  include/multiclass.h include/opinion.h include/act_wiz.h \
  include/handler.h
obj/hash.o: hash.c include/global.h include/bug.h include/comm.h \
  include/utils.h include/hash.h
obj/shop.o: shop.c include/global.h include/bug.h include/comm.h \
  include/handler.h include/db.h include/interpreter.h include/utils.h \
  include/act_comm.h include/multiclass.h include/act_social.h \
  include/act_wiz.h include/spec_procs.h include/weather.h \
  include/shop.h
obj/mkworld.o: extras/mkworld.c
obj/sign.o: extras/sign.c include/global.h include/bug.h
obj/bounce.o: extras/bounce.c
obj/genpasswd.o: extras/genpasswd.c
obj/randline.o: extras/randline.c
obj/scheduler.o: scheduler.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/scheduler.h
obj/stringmap.o: stringmap.c include/global.h include/bug.h include/sql.h \
  include/stringmap.h
obj/imc.o: imc.c
obj/reboot.o: reboot.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/interpreter.h include/db.h include/weather.h \
  include/sql.h include/i3.h include/reboot.h
obj/magic.o: magic.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/handler.h include/mudlimits.h include/db.h \
  include/spells.h include/spell_parser.h include/multiclass.h \
  include/fight.h include/opinion.h include/reception.h \
  include/magic_utils.h include/act_off.h include/act_obj.h \
  include/act_info.h include/weather.h
obj/spell_parser.o: spell_parser.c include/global.h include/bug.h \
  include/utils.h include/comm.h include/db.h include/interpreter.h \
  include/handler.h include/spells.h include/act_off.h include/random.h \
  include/multiclass.h include/fight.h include/act_info.h \
  include/spell_parser.h
obj/fight.o: fight.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/handler.h include/interpreter.h include/db.h \
  include/spells.h include/spell_parser.h include/mudlimits.h \
  include/random.h include/act_move.h include/reception.h \
  include/multiclass.h include/act_wiz.h include/act_skills.h \
  include/opinion.h include/spec_procs.h include/mob_actions.h \
  include/act_off.h include/fight.h
obj/2fa.o: 2fa.c include/global.h include/bug.h include/utils.h \
  include/base64.h include/2fa.h
obj/interpreter.o: interpreter.c include/global.h include/i3.h include/bug.h \
  include/comm.h include/version.h include/db.h include/utils.h \
  include/mudlimits.h include/act_comm.h include/act_info.h \
  include/act_move.h include/act_obj.h include/act_off.h \
  include/act_other.h include/act_skills.h include/act_social.h \
  include/act_wiz.h include/spells.h include/spell_parser.h \
  include/modify.h include/whod.h include/events.h include/random.h \
  include/board.h include/multiclass.h include/handler.h \
  include/reception.h include/tracking.h include/ban.h include/sql.h \
  include/reboot.h include/stringmap.h include/help.h \
  include/interpreter.h
obj/sha256.o: sha256.c include/sha256.h
obj/mob_actions.o: mob_actions.c include/global.h include/bug.h \
  include/utils.h include/act_obj.h include/act_off.h include/comm.h \
  include/db.h include/handler.h include/hash.h include/mudlimits.h \
  include/multiclass.h include/opinion.h include/spells.h \
  include/spell_parser.h include/fight.h include/trap.h \
  include/act_skills.h include/spec_procs.h include/tracking.h \
  include/mob_actions.h
obj/act_other.o: act_other.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/interpreter.h include/handler.h include/db.h \
  include/spells.h include/mudlimits.h include/spell_parser.h \
  include/reception.h include/multiclass.h include/fight.h \
  include/spec_procs.h include/act_skills.h include/act_comm.h \
  include/modify.h include/act_other.h
obj/act_comm.o: act_comm.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/interpreter.h include/handler.h include/db.h \
  include/spells.h include/mudlimits.h include/multiclass.h \
  include/board.h include/act_comm.h
obj/bug.o: bug.c include/global.h include/utils.h include/comm.h \
  include/multiclass.h include/db.h include/sql.h include/bug.h
obj/opinion.o: opinion.c include/global.h include/bug.h include/utils.h \
  include/spells.h include/comm.h include/db.h include/multiclass.h \
  include/opinion.h
obj/spells.o: spells.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/db.h include/interpreter.h include/handler.h \
  include/multiclass.h include/act_move.h include/weather.h \
  include/spells.h include/spell_parser.h include/fight.h
obj/tracking.o: tracking.c include/global.h include/bug.h include/utils.h \
  include/act_move.h include/comm.h include/db.h include/handler.h \
  include/hash.h include/interpreter.h include/multiclass.h \
  include/opinion.h include/spells.h include/fight.h include/modify.h \
  include/tracking.h
obj/act_info.o: act_info.c include/global.h include/bug.h include/version.h \
  include/utils.h include/comm.h include/interpreter.h include/handler.h \
  include/db.h include/spells.h include/mudlimits.h include/trap.h \
  include/hash.h include/spell_parser.h include/whod.h \
  include/multiclass.h include/modify.h include/act_wiz.h \
  include/act_skills.h include/spec_procs.h include/tracking.h \
  include/scheduler.h include/i3.h include/sql.h include/reboot.h \
  include/weather.h include/act_info.h
obj/act_skills.o: act_skills.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/db.h include/handler.h include/interpreter.h \
  include/multiclass.h include/spells.h include/spell_parser.h \
  include/fight.h include/spec_procs.h include/act_info.h \
  include/act_skills.h
obj/crc32.o: crc32.c include/crc32.h
obj/board.o: board.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/db.h include/interpreter.h include/multiclass.h \
  include/modify.h include/handler.h include/act_info.h include/sql.h \
  include/board.h
obj/act_off.o: act_off.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/interpreter.h include/handler.h include/db.h \
  include/spells.h include/mudlimits.h include/opinion.h \
  include/multiclass.h include/spec_procs.h include/fight.h \
  include/act_skills.h include/act_move.h include/spell_parser.h \
  include/act_info.h include/breath_weapons.h include/act_off.h
obj/act_obj.o: act_obj.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/interpreter.h include/handler.h include/db.h \
  include/spells.h include/trap.h include/spell_parser.h \
  include/multiclass.h include/mudlimits.h include/fight.h \
  include/act_info.h include/act_obj.h
obj/random.o: random.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/i3.h include/random.h
obj/events.o: events.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/interpreter.h include/handler.h include/db.h \
  include/spells.h include/mudlimits.h include/spell_parser.h \
  include/fight.h include/spec_procs.h include/opinion.h include/hash.h \
  include/events.h
obj/help.o: help.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/stringmap.h include/modify.h \
  include/multiclass.h include/sql.h include/help.h
obj/sql.o: sql.c include/global.h include/bug.h include/comm.h \
  include/version.h include/crc32.h include/interpreter.h \
  include/utils.h include/db.h include/weather.h include/ban.h \
  include/reboot.h include/rent.h include/stringmap.h include/help.h \
  include/whod.h include/board.h include/i3.h include/sql.h
obj/modify.o: modify.c include/global.h include/bug.h include/utils.h \
  include/interpreter.h include/handler.h include/db.h include/comm.h \
  include/multiclass.h include/modify.h
obj/i3.o: i3.c include/global.h include/sql.h include/bug.h include/utils.h \
  include/db.h include/multiclass.h include/comm.h include/modify.h \
  include/interpreter.h include/version.h include/stringmap.h \
  include/scheduler.h include/i3.h

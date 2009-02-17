# Must be GNU Make!

DATESTAMP	= $(shell /bin/date "+%y.%m.%d")
DEPLINE		= $(shell /bin/grep -n -e '^\#--- Comment for make deps ---' GNUmakefile | awk -F: '{print $$1}')

CC		= gcc #g++
MAKE		= make #gmake

BINDIR		= bin
IDENTDIR	= libident-0.19

NEED_CRYPT	= -lcrypt #Linux needs this
#NEED_COMPAT	= -lcompat #BSD needs this
SQL_INC		= -DUSE_PGSQL -I/usr/include/postgresql
SQL_LIB		= -L/usr/lib/postgresql -lpq -lecpg
ID_DEP		= $(IDENTDIR)/libident.a
ID_INC		= #
ID_LIB		= -L$(IDENTDIR) -lident

W_ERROR         = -Werror
W_ANSI          = #-pedantic
W_UBER          = -Wall
W_FORMAT        = -Wformat -Wformat-security -Wmissing-format-attribute
W_MESSY         = -Wmissing-braces -Wparentheses -Wshadow -Wredundant-decls
W_TYPE          = -Wcast-qual -Wcast-align -Wchar-subscripts -Wreturn-type -Wswitch -Wwrite-strings
W_EXTRA         = -Wunused -Wuninitialized #-Wunreachable-code
W_NITPICK       = -Wpointer-arith -Winline
ifeq ($(CC), gcc)
W_CONLY		= -Wmissing-declarations -Wmissing-prototypes -Wstrict-prototypes 
endif

W_FLAGS		= $(W_ERROR) $(W_ANSI) $(W_UBER) $(W_FORMAT) $(W_MESSY) $(W_TYPE) $(W_EXTRA) $(W_NITPICK) $(W_CONLY)

OPT_FLAG	= -pipe -O3 #-Wuninitialized

DEBUG_FLAG	= -ggdb
EFENCE		= #-lpthread -lefence
PROF_FLAG	= #-pg

INC		= -Iinclude $(SQL_INC) $(ID_INC)
LIB		= $(ID_LIB) $(NEED_CRYPT) $(NEED_COMPAT) $(EFENCE) $(SQL_LIB)

CFLAGS		= $(W_FLAGS) $(DEBUG_FLAG) $(OPT_FLAG) $(PROF_FLAG) $(INC)
LDFLAGS		= $(DEBUG_FLAG) $(OPT_FLAG) $(PROF_FLAG)

VER		= $(addprefix include/, version.h)
OLDV		= $(shell perl -ne '$$i = $$1 if /Version\s+\d\.(\d+)w3-alpha/i; END { $$i = 0 if !defined $$i; print "$$i\n";}' < $(VER))
VERSION		= $(shell echo $(OLDV) + 1 | bc)

tOBJ =	mob_actions.o spec_procs.o breath_weapons.o tracking.o \
	act_comm.o act_info.o act_move.o act_obj.o act_off.o \
	act_other.o act_skills.o act_social.o act_wiz.o \
	board.o bug.o comm.o constants.o db.o fight.o handler.o \
	hash.o interpreter.o limits.o magic.o magic_utils.o \
	modify.o multiclass.o opinion.o random.o reception.o \
	shop.o signals.o sound.o spell_parser.o \
	spells.o trap.o utils.o weather.o whod.o events.o \
	sql.o ban.o
OBJ =	$(addprefix obj/, $(tOBJ))

tIDOBJ		= id_close.o id_open.o id_parse.o \
		  id_query.o ident.o support.o version.o
IDOBJ		= $(addprefix obj/, $(tIDOBJ))

all : version depend bin

bin : $(BINDIR)/sign $(BINDIR)/bounce $(BINDIR)/genpasswd $(BINDIR)/mkworld $(BINDIR)/wileymud 

$(BINDIR)/wileymud : $(ID_DEP) $(OBJ)
	@echo Heave Ho!  Load the beastie!
	$(CC) $(LDFLAGS) -o $@ $(OBJ) $(LIB)
	@ls -l $(BINDIR)

$(BINDIR)/mkworld : obj/mkworld.o
	@echo Make the world editing program...
	$(CC) $(LDFLAGS) -o $@ $< $(LIB)

$(BINDIR)/sign : obj/sign.o
	@echo Make the downtime notice program...
	$(CC) $(LDFLAGS) -o $@ $< $(LIB)

$(BINDIR)/genpasswd : obj/genpasswd.o
	@echo Make the password generation program...
	$(CC) $(LDFLAGS) -o $@ $< $(LIB)

$(BINDIR)/bounce : obj/bounce.o
	@echo Make the port bouncer program...
	$(CC) $(LDFLAGS) -o $@ $< $(LIB)

$(ID_DEP): $(IDOBJ)
	(cd $(IDENTDIR);$(MAKE) linux)

.PHONY : depend

depend :
	@echo 'Building dependancies'
	@find . -name \*.c -a -type f -print0 | grep -z -Z -v attic | xargs -0 -P 1 -r $(CC) $(CFLAGS) -MM >.deps
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
	@echo "#define VERSION_STR \\" >> $(VER)
	@echo "\"\r\n*** Welcome to WileyMUD III, Quixadhal's Version 0.$(VERSION)w3-alpha ($(DATESTAMP)) ***\r\n\"" >> $(VER)
	@echo '#endif /* _VERSION_H */' >> $(VER)
	@echo New version file generated.

.PHONY : clean

clean :
	@echo -n Wiping away intermediate clutter...
	@find . -name \*.o -a -type f -print0 | xargs -0 -P 1 -r rm -f
	@rm -f tags
	@echo done.

.PHONY : tags

tags :
	@echo -n Making tag files...
	@find . -name \*.[ch] -a -type f -print0 | grep -z -Z -v attic | xargs -0 -P 1 -r ctags
	@echo done.

%c : %pgc
	ecpg $(INC) -c $< -o $@

obj/%o : %c
	$(CC) $(CFLAGS) -c $< -o $@

obj/sign.o obj/mkworld.o obj/genpasswd.o obj/bounce.o : obj/%o : aux/%c
	$(CC) $(CFLAGS) -c $< -o $@

#--- Comment for make deps ---
obj/breath_weapons.o: breath_weapons.c include/global.h include/bug.h \
  include/utils.h include/act_off.h include/comm.h include/db.h \
  include/mudlimits.h include/multiclass.h include/spells.h \
  include/breath_weapons.h
obj/sound.o: sound.c include/global.h include/bug.h include/comm.h \
  include/interpreter.h include/handler.h include/db.h include/spells.h \
  include/trap.h include/utils.h include/sound.h
obj/random.o: random.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/random.h
obj/act_obj.o: act_obj.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/interpreter.h include/handler.h include/db.h \
  include/spells.h include/trap.h include/constants.h \
  include/spell_parser.h include/multiclass.h include/mudlimits.h \
  include/fight.h include/act_info.h include/act_obj.h
obj/weather.o: weather.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/handler.h include/interpreter.h include/db.h \
  include/weather.h
obj/spell_parser.o: spell_parser.c include/global.h include/bug.h \
  include/utils.h include/comm.h include/db.h include/interpreter.h \
  include/handler.h include/spells.h include/constants.h \
  include/act_off.h include/random.h include/multiclass.h include/fight.h \
  include/act_info.h include/spell_parser.h
obj/db.o: db.c include/global.h include/bug.h include/utils.h include/comm.h \
  include/handler.h include/mudlimits.h include/opinion.h include/hash.h \
  include/constants.h include/spells.h include/spell_parser.h \
  include/reception.h include/weather.h include/modify.h include/fight.h \
  include/act_social.h include/spec_procs.h include/multiclass.h \
  include/board.h include/interpreter.h include/ban.h include/db.h
obj/events.o: events.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/interpreter.h include/handler.h include/db.h \
  include/spells.h include/mudlimits.h include/constants.h \
  include/spell_parser.h include/fight.h include/spec_procs.h \
  include/opinion.h include/hash.h include/events.h
obj/multiclass.o: multiclass.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/db.h include/interpreter.h include/handler.h \
  include/spells.h include/mudlimits.h include/opinion.h \
  include/constants.h include/multiclass.h
obj/whod.o: whod.c include/version.h include/global.h include/bug.h \
  include/db.h include/comm.h include/utils.h include/interpreter.h \
  include/multiclass.h include/whod.h
obj/constants.o: constants.c include/global.h include/bug.h \
  include/mudlimits.h include/trap.h include/constants.h
obj/limits.o: limits.c include/global.h include/bug.h include/spells.h \
  include/comm.h include/db.h include/spell_parser.h include/constants.h \
  include/utils.h include/multiclass.h include/fight.h \
  include/reception.h include/interpreter.h include/handler.h \
  include/act_obj.h include/act_other.h include/mudlimits.h
obj/spells.o: spells.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/db.h include/interpreter.h include/handler.h \
  include/constants.h include/multiclass.h include/act_move.h \
  include/spells.h include/spell_parser.h include/fight.h
obj/board.o: board.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/db.h include/interpreter.h include/multiclass.h \
  include/modify.h include/handler.h include/act_info.h include/board.h
obj/opinion.o: opinion.c include/global.h include/bug.h include/utils.h \
  include/spells.h include/constants.h include/comm.h include/db.h \
  include/multiclass.h include/opinion.h
obj/signals.o: signals.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/whod.h include/signals.h
obj/act_skills.o: act_skills.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/constants.h include/db.h include/handler.h \
  include/interpreter.h include/multiclass.h include/spells.h \
  include/spell_parser.h include/fight.h include/spec_procs.h \
  include/act_info.h include/act_skills.h
obj/act_social.o: act_social.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/interpreter.h include/handler.h include/db.h \
  include/spells.h include/multiclass.h include/act_social.h
obj/utils.o: utils.c include/global.h include/bug.h include/spells.h \
  include/constants.h include/db.h include/opinion.h include/comm.h \
  include/hash.h include/multiclass.h include/handler.h include/fight.h \
  include/act_info.h include/reception.h include/act_off.h \
  include/magic_utils.h include/mudlimits.h include/act_skills.h \
  include/utils.h
obj/magic.o: magic.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/handler.h include/mudlimits.h include/db.h \
  include/constants.h include/spells.h include/spell_parser.h \
  include/multiclass.h include/fight.h include/opinion.h \
  include/reception.h include/magic_utils.h include/act_off.h \
  include/act_obj.h include/act_info.h
obj/modify.o: modify.c include/global.h include/bug.h include/utils.h \
  include/interpreter.h include/handler.h include/db.h include/comm.h \
  include/multiclass.h include/modify.h
obj/act_comm.o: act_comm.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/interpreter.h include/handler.h include/db.h \
  include/spells.h include/constants.h include/mudlimits.h \
  include/multiclass.h include/act_comm.h
obj/spec_procs.o: spec_procs.c include/global.h include/bug.h include/utils.h \
  include/act_comm.h include/act_info.h include/act_move.h \
  include/act_obj.h include/act_off.h include/act_social.h \
  include/act_wiz.h include/board.h include/comm.h include/constants.h \
  include/db.h include/handler.h include/hash.h include/interpreter.h \
  include/mudlimits.h include/modify.h include/multiclass.h \
  include/opinion.h include/reception.h include/shop.h include/spells.h \
  include/spell_parser.h include/fight.h include/breath_weapons.h \
  include/mob_actions.h include/tracking.h include/spec_procs.h
obj/act_other.o: act_other.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/interpreter.h include/handler.h include/db.h \
  include/spells.h include/mudlimits.h include/constants.h \
  include/spell_parser.h include/reception.h include/multiclass.h \
  include/fight.h include/spec_procs.h include/act_skills.h \
  include/act_comm.h include/modify.h include/act_other.h
obj/act_move.o: act_move.c include/global.h include/utils.h include/bug.h \
  include/comm.h include/interpreter.h include/handler.h include/db.h \
  include/spells.h include/trap.h include/constants.h include/whod.h \
  include/multiclass.h include/fight.h include/reception.h \
  include/magic_utils.h include/spell_parser.h include/act_info.h \
  include/act_skills.h include/act_move.h
obj/reception.o: reception.c include/global.h include/bug.h include/comm.h \
  include/handler.h include/db.h include/interpreter.h include/utils.h \
  include/spells.h include/multiclass.h include/act_social.h \
  include/spec_procs.h include/reception.h
obj/genpasswd.o: aux/genpasswd.c
obj/sign.o: aux/sign.c include/global.h include/bug.h
obj/bounce.o: aux/bounce.c
obj/mkworld.o: aux/mkworld.c
obj/dump_afk.o: convert/dump_afk.c convert/include/structs.h \
  convert/include/main.h convert/include/utils.h \
  convert/include/parse_wiley.h convert/include/dump_afk.h \
  convert/include/afk_skills.h
obj/parse_wiley.o: convert/parse_wiley.c convert/include/bug.h \
  convert/include/structs.h convert/include/main.h \
  convert/include/utils.h convert/include/make_index.h \
  convert/include/parse_wiley.h
obj/dump_report.o: convert/dump_report.c convert/include/structs.h \
  convert/include/main.h convert/include/utils.h \
  convert/include/parse_wiley.h convert/include/dump_report.h
obj/make_index.o: convert/make_index.c convert/include/bug.h \
  convert/include/structs.h convert/include/main.h \
  convert/include/utils.h convert/include/make_index.h
obj/getopt.o: convert/getopt.c
obj/utils.o: convert/utils.c convert/include/bug.h convert/include/structs.h \
  convert/include/main.h convert/include/utils.h
obj/formats.o: convert/formats.c convert/include/formats.h
obj/main.o: convert/main.c convert/include/getopt.h convert/include/bug.h \
  convert/include/formats.h convert/include/structs.h \
  convert/include/main.h convert/include/utils.h \
  convert/include/make_index.h convert/include/parse_wiley.h \
  convert/include/dump_map.h convert/include/dump_report.h \
  convert/include/dump_isles.h convert/include/dump_fr.h \
  convert/include/dump_afk.h
obj/bug.o: convert/bug.c convert/include/bug.h
obj/dump_map.o: convert/dump_map.c convert/include/structs.h \
  convert/include/main.h convert/include/utils.h \
  convert/include/parse_wiley.h convert/include/dump_map.h
obj/dump_isles.o: convert/dump_isles.c convert/include/structs.h \
  convert/include/main.h convert/include/utils.h \
  convert/include/parse_wiley.h convert/include/dump_isles.h
obj/dump_fr.o: convert/dump_fr.c convert/include/structs.h \
  convert/include/main.h convert/include/utils.h \
  convert/include/parse_wiley.h convert/include/dump_fr.h
obj/getopt1.o: convert/getopt1.c convert/include/getopt.h
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
  include/reception.h include/tracking.h include/ban.h \
  include/interpreter.h
obj/fight.o: fight.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/handler.h include/interpreter.h include/db.h \
  include/constants.h include/spells.h include/spell_parser.h \
  include/mudlimits.h include/random.h include/act_move.h \
  include/reception.h include/multiclass.h include/act_wiz.h \
  include/act_skills.h include/opinion.h include/spec_procs.h \
  include/mob_actions.h include/act_off.h include/fight.h
obj/trap.o: trap.c include/global.h include/bug.h include/utils.h \
  include/spells.h include/comm.h include/db.h include/opinion.h \
  include/constants.h include/fight.h include/reception.h \
  include/spell_parser.h include/multiclass.h include/handler.h \
  include/act_info.h include/trap.h
obj/comm.o: comm.c include/global.h include/bug.h include/utils.h \
  include/interpreter.h include/handler.h include/db.h include/modify.h \
  include/whod.h include/multiclass.h include/weather.h \
  include/mudlimits.h include/spells.h include/spell_parser.h \
  include/sound.h include/fight.h include/mob_actions.h \
  include/act_other.h include/signals.h include/sql.h include/ban.h \
  include/comm.h
obj/magic_utils.o: magic_utils.c include/global.h include/bug.h \
  include/utils.h include/comm.h include/db.h include/spells.h \
  include/handler.h include/mudlimits.h include/fight.h \
  include/spell_parser.h include/multiclass.h include/magic_utils.h
obj/act_off.o: act_off.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/interpreter.h include/handler.h include/db.h \
  include/spells.h include/mudlimits.h include/opinion.h \
  include/multiclass.h include/constants.h include/spec_procs.h \
  include/fight.h include/act_skills.h include/act_move.h \
  include/spell_parser.h include/act_info.h include/breath_weapons.h \
  include/act_off.h
obj/shop.o: shop.c include/global.h include/bug.h include/comm.h \
  include/handler.h include/db.h include/interpreter.h include/utils.h \
  include/constants.h include/act_comm.h include/multiclass.h \
  include/act_social.h include/act_wiz.h include/spec_procs.h \
  include/shop.h
obj/act_wiz.o: act_wiz.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/interpreter.h include/handler.h include/db.h \
  include/spells.h include/mudlimits.h include/constants.h \
  include/spell_parser.h include/board.h include/whod.h \
  include/reception.h include/spec_procs.h include/multiclass.h \
  include/act_skills.h include/act_info.h include/fight.h include/hash.h \
  include/weather.h include/modify.h include/act_wiz.h
obj/act_info.o: act_info.c include/global.h include/bug.h include/version.h \
  include/utils.h include/comm.h include/interpreter.h include/handler.h \
  include/db.h include/spells.h include/mudlimits.h include/trap.h \
  include/hash.h include/constants.h include/spell_parser.h \
  include/whod.h include/multiclass.h include/modify.h include/act_wiz.h \
  include/act_skills.h include/spec_procs.h include/tracking.h \
  include/act_info.h
obj/mob_actions.o: mob_actions.c include/global.h include/bug.h \
  include/utils.h include/act_obj.h include/act_off.h include/comm.h \
  include/constants.h include/db.h include/handler.h include/hash.h \
  include/mudlimits.h include/multiclass.h include/opinion.h \
  include/spells.h include/spell_parser.h include/fight.h include/trap.h \
  include/act_skills.h include/spec_procs.h include/tracking.h \
  include/mob_actions.h
obj/handler.o: handler.c include/global.h include/bug.h include/utils.h \
  include/comm.h include/db.h include/interpreter.h include/spells.h \
  include/spell_parser.h include/constants.h include/fight.h \
  include/modify.h include/multiclass.h include/opinion.h \
  include/act_wiz.h include/handler.h
obj/id_open.o: libident-0.19/id_open.c libident-0.19/ident.h
obj/id_parse.o: libident-0.19/id_parse.c libident-0.19/ident.h
obj/ident.o: libident-0.19/ident.c libident-0.19/ident.h
obj/ident-tester.o: libident-0.19/ident-tester.c libident-0.19/ident.h
obj/lookup-tester.o: libident-0.19/lookup-tester.c libident-0.19/ident.h
obj/id_close.o: libident-0.19/id_close.c libident-0.19/ident.h
obj/id_query.o: libident-0.19/id_query.c libident-0.19/ident.h
obj/version.o: libident-0.19/version.c
obj/support.o: libident-0.19/support.c libident-0.19/ident.h
obj/tracking.o: tracking.c include/global.h include/bug.h include/utils.h \
  include/act_move.h include/comm.h include/constants.h include/db.h \
  include/handler.h include/hash.h include/interpreter.h \
  include/multiclass.h include/opinion.h include/spells.h include/fight.h \
  include/tracking.h

# Must be GNU Make!

CC          = gcc #g++
MACHINE     = #-mcpu=pentium2 -march=pentium2

NEED_CRYPT  = -lcrypt #Linux needs this
#NEED_COMPAT = -lcompat #BSD needs this

W_FORMAT    = -Wformat -Wformat-security #-Wmissing-format-attribute
W_NITPICK   = -Wpointer-arith -Winline
W_MESSY     = -Wmissing-braces -Wparentheses -Wshadow #-Wredundant-decls
W_FUNC      = #-Wstrict-prototypes #-Wmissing-prototypes
W_TYPE      = -Wcast-qual -Wcast-align -Wchar-subscripts -Wreturn-type -Wswitch #-Wwrite-strings
W_EXTRA     = -Wunused #-Wunreachable-code
W_C_ONLY    = -Wmissing-declarations -Wimplicit-int -Wimplicit-function-declaration #-Wsequence-point -Wnonnull
W_ANSI      = #-pedantic
W_UBER      = -Wall #-W
W_ERROR     = -Werror

W_ALL       = $(W_ANSI) $(W_UBER) $(W_FORMAT) $(W_MESSY) $(W_FUNC) $(W_TYPE) $(W_EXTRA) $(W_NITPICK) $(W_ERROR)
W_CLANG     = $(W_C_ONLY)

W_FLAGS     = $(W_ALL) $(W_CLANG)

OPT_FLAG    = -pipe -O3 #-Wuninitialized

DEBUG_FLAG  = -ggdb
EFENCE      = #-lpthread -lefence
PROF_FLAG   = #-pg

CFLAGS      = $(MACHINE) $(W_FLAGS) $(DEBUG_FLAG) $(OPT_FLAG) $(PROF_FLAG) -Iinclude
LDFLAGS     = $(DEBUG_FLAG) $(OPT_FLAG) $(PROF_FLAG)

SRC =	bug.c utils.c make_index.c main.c formats.c \
	getopt.c getopt1.c \
	parse_wiley.c \
	dump_report.c dump_map.c dump_isles.c dump_fr.c dump_afk.c dump_smaug.c
OBJ =	$(addprefix obj/, $(SRC:.c=.o))
HDRS =	bug.h utils.h make_index.h main.h formats.h getopt.h \
	parse_wiley.h \
	dump_report.h dump_map.h dump_isles.h dump_fr.h dump_afk.h dump_smaug.h
HDR =	$(addprefix include/, $(HDRS))
LIB =	$(NEED_CRYPT) $(NEED_COMPAT) $(EFENCE)
AUX = GNUmakefile

all : tests

#.deps/%.d: %.c
#	set -e; $(CC) -M $(CFLAGS) $< \
#	        | sed 's!\($*\)\.o[ :]*!\1.o $@ : !g' > $@; \
#	        [ -s $@ ] || rm -f $@
#
#include $(addprefix .deps/, $(SRC:.c=.d))

tests : run.log

run.log : convert
	@echo Starting run at `date`...
	@echo Runtime log of WileyMUD Conversion/Parsing >run.log
	@date >>run.log
	@echo "" >>run.log
	@./convert -i wiley -o all 2>&1 | tee -a run.log
	@echo "" >>run.log
	@date >>run.log
	@echo Completed at `date`

convert : $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $(OBJ) $(LIB)

$(OBJ) : obj/%o : %c
	$(CC) $(CFLAGS) -c $< -o $@

clean :
	rm -f *.idx *.bak *.out *.log
	rm -rf obj
	mkdir obj
	rm -rf output
	rm -f convert
	@find . -name \*.d | xargs rm -f


#!/usr/local/bin/gmake

CC = gcc
CFLAGS = -O3
#CFLAGS = -g -Dsun3 -m68020 -fstrength-reduce -fvolatile 
LDFLAGS = -s
LIB =
SRC =	db.c comm.c act.comm.c act.informative.c act.move.c act.obj1.c \
	act.obj2.c \
	act.offensive.c act.other.c act.social.c act.wizard.c handler.c \
	interpreter.c utility.c spec_assign.c shop.c limits.c mobact.c \
	fight.c modify.c weather.c spells1.c spells2.c spells3.c \
	spell_parser.c reception.c constants.c spec_procs.c signals.c \
	board.c magic.c magic2.c magic3.c skills.c Opinion.c Trap.c \
	magicutils.c multiclass.c hash.c Sound.c whod.c 
OBJ = $(SRC:.c=.o)
HDR = structs.h utils.h comm.h interpreter.h db.h
AUX = GNUmakefile lex.yy.c dbparser.l

all : dmserver

dmserver : $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $(OBJ) $(LIB)

$(OBJ) : %o : %c
	$(CC) $(CFLAGS) -c $< -o $@

lex.yy.c : dbparser.l
	flex -f $<

clean :
	rm -f *.o *.bak
	rm -f dmserver
	rm -f printout

backup :
	tar -zcvf backup.tar.gz $(SRC) $(HDR) $(AUX)

printout : $(SRC) $(HDR)
	/usr/5bin/pr -l80 -w96 -f -n5 $(HDR) $(SRC) > $@

depend : lex.yy.c $(SRC) $(HDR)
	makedepend -fGNUmakefile -- $(CFLAGS) -- $(SRC)

# DO NOT DELETE THIS LINE -- make depend depends on it.

comm.o: /usr/include/errno.h /usr/include/sys/errno.h /usr/include/stdio.h
comm.o: /usr/include/ctype.h /usr/include/sys/types.h
comm.o: /usr/include/sys/stdtypes.h /usr/include/sys/sysmacros.h
comm.o: /usr/include/sys/socket.h /usr/include/sys/wait.h
comm.o: /usr/include/netinet/in.h /usr/include/netdb.h /usr/include/string.h
comm.o: /usr/include/sys/time.h /usr/include/sys/time.h /usr/include/fcntl.h
comm.o: /usr/include/sys/fcntlcom.h /usr/include/sys/stat.h
comm.o: /usr/include/signal.h /usr/include/sys/signal.h
comm.o: /usr/include/vm/faultcode.h /usr/include/sys/un.h
comm.o: /usr/include/sys/resource.h structs.h utils.h comm.h interpreter.h
comm.o: handler.h db.h
act.comm.o: /usr/include/stdio.h /usr/include/string.h
act.comm.o: /usr/include/sys/stdtypes.h structs.h /usr/include/sys/types.h
act.comm.o: /usr/include/sys/sysmacros.h utils.h comm.h interpreter.h
act.comm.o: handler.h db.h spells.h
act.informative.o: /usr/include/stdio.h /usr/include/string.h
act.informative.o: /usr/include/sys/stdtypes.h /usr/include/ctype.h
act.informative.o: /usr/include/sys/time.h structs.h /usr/include/sys/types.h
act.informative.o: /usr/include/sys/sysmacros.h utils.h comm.h interpreter.h
act.informative.o: handler.h db.h spells.h limits.h race.h trap.h hash.h
act.move.o: /usr/include/stdio.h /usr/include/string.h
act.move.o: /usr/include/sys/stdtypes.h structs.h /usr/include/sys/types.h
act.move.o: /usr/include/sys/sysmacros.h utils.h comm.h interpreter.h
act.move.o: handler.h db.h spells.h trap.h
act.obj1.o: /usr/include/stdio.h /usr/include/string.h
act.obj1.o: /usr/include/sys/stdtypes.h structs.h /usr/include/sys/types.h
act.obj1.o: /usr/include/sys/sysmacros.h utils.h comm.h interpreter.h
act.obj1.o: handler.h db.h spells.h trap.h
act.obj2.o: /usr/include/stdio.h /usr/include/string.h
act.obj2.o: /usr/include/sys/stdtypes.h /usr/include/assert.h structs.h
act.obj2.o: /usr/include/sys/types.h /usr/include/sys/sysmacros.h utils.h
act.obj2.o: comm.h interpreter.h handler.h db.h spells.h
act.offensive.o: /usr/include/stdio.h /usr/include/string.h
act.offensive.o: /usr/include/sys/stdtypes.h structs.h
act.offensive.o: /usr/include/sys/types.h /usr/include/sys/sysmacros.h
act.offensive.o: utils.h comm.h interpreter.h handler.h db.h spells.h
act.offensive.o: limits.h opinion.h race.h multiclass.h
act.other.o: /usr/include/stdio.h /usr/include/string.h
act.other.o: /usr/include/sys/stdtypes.h /usr/include/ctype.h structs.h
act.other.o: /usr/include/sys/types.h /usr/include/sys/sysmacros.h utils.h
act.other.o: comm.h interpreter.h handler.h db.h spells.h limits.h
act.social.o: /usr/include/stdio.h /usr/include/string.h
act.social.o: /usr/include/sys/stdtypes.h structs.h /usr/include/sys/types.h
act.social.o: /usr/include/sys/sysmacros.h utils.h comm.h interpreter.h
act.social.o: handler.h db.h spells.h
act.wizard.o: /usr/include/stdio.h /usr/include/string.h
act.wizard.o: /usr/include/sys/stdtypes.h /usr/include/ctype.h structs.h
act.wizard.o: /usr/include/sys/types.h /usr/include/sys/sysmacros.h utils.h
act.wizard.o: comm.h interpreter.h handler.h db.h spells.h limits.h race.h
handler.o: /usr/include/string.h /usr/include/sys/stdtypes.h
handler.o: /usr/include/stdio.h /usr/include/ctype.h /usr/include/assert.h
handler.o: structs.h /usr/include/sys/types.h /usr/include/sys/sysmacros.h
handler.o: utils.h comm.h db.h handler.h interpreter.h spells.h
db.o: /usr/include/stdio.h /usr/include/string.h /usr/include/sys/stdtypes.h
db.o: /usr/include/ctype.h /usr/include/sys/time.h structs.h
db.o: /usr/include/sys/types.h /usr/include/sys/sysmacros.h utils.h db.h
db.o: comm.h handler.h limits.h race.h opinion.h hash.h spells.h myerrors.h
db.o: lex.yy.c
interpreter.o: /usr/include/string.h /usr/include/sys/stdtypes.h
interpreter.o: /usr/include/ctype.h /usr/include/stdio.h
interpreter.o: /usr/include/arpa/telnet.h structs.h /usr/include/sys/types.h
interpreter.o: /usr/include/sys/sysmacros.h comm.h interpreter.h db.h utils.h
interpreter.o: limits.h race.h
utility.o: /usr/include/stdio.h /usr/include/malloc.h /usr/include/string.h
utility.o: /usr/include/sys/stdtypes.h /usr/include/assert.h structs.h
utility.o: /usr/include/sys/types.h /usr/include/sys/sysmacros.h
utility.o: /usr/include/sys/time.h utils.h spells.h race.h db.h opinion.h
utility.o: comm.h hash.h
spec_assign.o: /usr/include/stdio.h structs.h /usr/include/sys/types.h
spec_assign.o: /usr/include/sys/stdtypes.h /usr/include/sys/sysmacros.h db.h
shop.o: /usr/include/stdio.h /usr/include/string.h
shop.o: /usr/include/sys/stdtypes.h structs.h /usr/include/sys/types.h
shop.o: /usr/include/sys/sysmacros.h comm.h handler.h db.h interpreter.h
shop.o: utils.h
limits.o: /usr/include/stdio.h /usr/include/assert.h structs.h
limits.o: /usr/include/sys/types.h /usr/include/sys/stdtypes.h
limits.o: /usr/include/sys/sysmacros.h limits.h utils.h spells.h comm.h
limits.o: race.h
mobact.o: /usr/include/stdio.h structs.h /usr/include/sys/types.h
mobact.o: /usr/include/sys/stdtypes.h /usr/include/sys/sysmacros.h utils.h
mobact.o: handler.h db.h comm.h opinion.h trap.h
fight.o: /usr/include/stdio.h /usr/include/string.h
fight.o: /usr/include/sys/stdtypes.h /usr/include/assert.h structs.h
fight.o: /usr/include/sys/types.h /usr/include/sys/sysmacros.h utils.h comm.h
fight.o: handler.h interpreter.h db.h spells.h
modify.o: /usr/include/stdio.h /usr/include/ctype.h /usr/include/string.h
modify.o: /usr/include/sys/stdtypes.h /usr/include/sys/time.h structs.h
modify.o: /usr/include/sys/types.h /usr/include/sys/sysmacros.h utils.h
modify.o: interpreter.h handler.h db.h comm.h
weather.o: /usr/include/stdio.h /usr/include/string.h
weather.o: /usr/include/sys/stdtypes.h structs.h /usr/include/sys/types.h
weather.o: /usr/include/sys/sysmacros.h utils.h comm.h handler.h
weather.o: interpreter.h db.h
spells1.o: /usr/include/stdio.h structs.h /usr/include/sys/types.h
spells1.o: /usr/include/sys/stdtypes.h /usr/include/sys/sysmacros.h utils.h
spells1.o: comm.h db.h interpreter.h spells.h handler.h s_list.h
spells2.o: /usr/include/stdio.h structs.h /usr/include/sys/types.h
spells2.o: /usr/include/sys/stdtypes.h /usr/include/sys/sysmacros.h utils.h
spells2.o: comm.h db.h interpreter.h spells.h handler.h poly.h
spells3.o: /usr/include/stdio.h /usr/include/strings.h structs.h
spells3.o: /usr/include/sys/types.h /usr/include/sys/stdtypes.h
spells3.o: /usr/include/sys/sysmacros.h utils.h comm.h db.h interpreter.h
spells3.o: spells.h handler.h poly.h
spell_parser.o: /usr/include/stdio.h /usr/include/assert.h structs.h
spell_parser.o: /usr/include/sys/types.h /usr/include/sys/stdtypes.h
spell_parser.o: /usr/include/sys/sysmacros.h utils.h comm.h db.h
spell_parser.o: interpreter.h spells.h handler.h
reception.o: /usr/include/stdio.h /usr/include/sys/time.h
reception.o: /usr/include/sys/time.h structs.h /usr/include/sys/types.h
reception.o: /usr/include/sys/stdtypes.h /usr/include/sys/sysmacros.h comm.h
reception.o: handler.h db.h interpreter.h utils.h spells.h reception.h
constants.o: structs.h /usr/include/sys/types.h /usr/include/sys/stdtypes.h
constants.o: /usr/include/sys/sysmacros.h limits.h trap.h
spec_procs.o: /usr/include/stdio.h /usr/include/string.h
spec_procs.o: /usr/include/sys/stdtypes.h /usr/include/ctype.h structs.h
spec_procs.o: /usr/include/sys/types.h /usr/include/sys/sysmacros.h utils.h
spec_procs.o: comm.h interpreter.h handler.h db.h spells.h limits.h s_list.h
spec_procs.o: opinion.h hash.h
signals.o: /usr/include/signal.h /usr/include/sys/signal.h
signals.o: /usr/include/vm/faultcode.h /usr/include/stdio.h
signals.o: /usr/include/sys/time.h /usr/include/sys/time.h structs.h
signals.o: /usr/include/sys/types.h /usr/include/sys/stdtypes.h
signals.o: /usr/include/sys/sysmacros.h utils.h
board.o: /usr/include/string.h /usr/include/sys/stdtypes.h
board.o: /usr/include/stdio.h /usr/include/ctype.h structs.h
board.o: /usr/include/sys/types.h /usr/include/sys/sysmacros.h utils.h comm.h
board.o: db.h
magic.o: /usr/include/stdio.h /usr/include/assert.h structs.h
magic.o: /usr/include/sys/types.h /usr/include/sys/stdtypes.h
magic.o: /usr/include/sys/sysmacros.h utils.h comm.h spells.h handler.h
magic.o: limits.h db.h
magic2.o: /usr/include/stdio.h /usr/include/assert.h structs.h
magic2.o: /usr/include/sys/types.h /usr/include/sys/stdtypes.h
magic2.o: /usr/include/sys/sysmacros.h utils.h comm.h spells.h handler.h
magic2.o: limits.h db.h race.h
magic3.o: /usr/include/stdio.h /usr/include/assert.h structs.h
magic3.o: /usr/include/sys/types.h /usr/include/sys/stdtypes.h
magic3.o: /usr/include/sys/sysmacros.h utils.h comm.h spells.h handler.h
magic3.o: limits.h db.h race.h
skills.o: structs.h /usr/include/sys/types.h /usr/include/sys/stdtypes.h
skills.o: /usr/include/sys/sysmacros.h utils.h race.h spells.h comm.h
skills.o: handler.h
Opinion.o: /usr/include/stdio.h /usr/include/string.h
Opinion.o: /usr/include/sys/stdtypes.h /usr/include/ctype.h
Opinion.o: /usr/include/sys/time.h structs.h /usr/include/sys/types.h
Opinion.o: /usr/include/sys/sysmacros.h utils.h spells.h race.h opinion.h
Opinion.o: db.h
Trap.o: structs.h /usr/include/sys/types.h /usr/include/sys/stdtypes.h
Trap.o: /usr/include/sys/sysmacros.h utils.h spells.h trap.h comm.h
magicutils.o: /usr/include/stdio.h /usr/include/assert.h structs.h
magicutils.o: /usr/include/sys/types.h /usr/include/sys/stdtypes.h
magicutils.o: /usr/include/sys/sysmacros.h utils.h comm.h spells.h handler.h
magicutils.o: limits.h
multiclass.o: /usr/include/stdio.h /usr/include/string.h
multiclass.o: /usr/include/sys/stdtypes.h structs.h /usr/include/sys/types.h
multiclass.o: /usr/include/sys/sysmacros.h utils.h comm.h interpreter.h
multiclass.o: handler.h spells.h limits.h opinion.h race.h multiclass.h
hash.o: /usr/include/stdio.h hash.h
Sound.o: /usr/include/stdio.h /usr/include/string.h
Sound.o: /usr/include/sys/stdtypes.h structs.h /usr/include/sys/types.h
Sound.o: /usr/include/sys/sysmacros.h utils.h comm.h interpreter.h handler.h
Sound.o: db.h spells.h trap.h
whod.o: /usr/include/sys/types.h /usr/include/sys/stdtypes.h
whod.o: /usr/include/sys/sysmacros.h /usr/include/sys/socket.h
whod.o: /usr/include/sys/un.h /usr/include/netdb.h /usr/include/netinet/in.h
whod.o: /usr/include/sys/time.h /usr/include/sys/time.h /usr/include/signal.h
whod.o: /usr/include/sys/signal.h /usr/include/vm/faultcode.h
whod.o: /usr/include/stdio.h structs.h utils.h

CC = gcc

W_FLAGS = -Wall -Werror -Wformat-security -Winline -Wstrict-prototypes -Wnested-externs -Wshadow -Wpointer-arith -Wcast-align -Wcast-qual -Wredundant-decls -pedantic

CFLAGS = -g2 -Os -DIMCSTANDALONE $(W_FLAGS)
LFLAGS = -g2

all:    imc.o sha256.o
	rm -f imc
	$(CC) $(CFLAGS) -o imc imc.o sha256.o $(LFLAGS)

clean:
	rm -f *.o imc core
	make all

purge:
	rm -f *.o imc core

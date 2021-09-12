CC=gcc
IDIR=../include
CFLAGS=-I$(IDIR) -Wall -g -O0

ODIR=obj
LDIR=../lib

LIBS=-lSDL2main -lSDL2 -lSDL2_image

_DEPS = game.h graphics.h ia.h gamestate.h tools.h bitboard.h
DEPS = $(patsubst %,$(ODIR)/%,$(_OBJ))

_OBJ = main.o game.o graphics.o ia.o gamestate.o tools.o bitboard.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

main: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~

# ifndef TOOLS_H
# define TOOLS_H

# include "gamestate.h"
# include "bitboard.h"

void printThreats(int threats[]);
void indexToName(int index, char result[3]);
int nameToIndex(const char name[3]);
void printThreats2(int threats[], int size);
void printBoard(gamestate game);
void printBits(bitboard bits);
int startWith(const char shorter[], const char longer[]);
void parseConfig(const char command[], gamestate* game);
void parseSearch(const char command[], gamestate* game);
int signe(int x);
void loadConfiguration(const char config[], gamestate* game);
void printConfiguration(gamestate game);

# endif

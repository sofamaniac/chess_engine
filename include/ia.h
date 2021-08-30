# ifndef IA_H
# define IA_H

# include <SDL2/SDL.h>
# include "game.h"
# include "bitboard.h"

# define min(X, Y) ((X) < (Y) ? (X) : (Y))

typedef struct moveList {
	int start;
	int end;
	struct moveList* next;
} moveList;

int threatened2(gamestate* game, int target, bitboard* result);
bitboard accessible(gamestate* game, int from, int color);
void threatsSliding(gamestate* game, int position, int start, int end, int color, bitboard* result);

int createAllMoves2(gamestate* game, moveList* list);
void freeMoveList(moveList* list);
int depthSearch(gamestate game, int depth);
int isInBound(int x, int y);
void printMove(int start, int end, char prom, int nb);
# endif

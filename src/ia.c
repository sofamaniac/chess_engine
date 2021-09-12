# include "ia.h"
# include <SDL2/SDL.h>
# include "game.h"
# include "gamestate.h"
# include "tools.h"
# include "gamestate.h"

extern const bitboard pawnAttackMask[2][BOARD_SIZE];
extern const bitboard pawnPositionMask[2][BOARD_SIZE];
extern const bitboard kingAttackMask[BOARD_SIZE];
extern const bitboard kingPositionMask[BOARD_SIZE];
extern const bitboard knightAttackMask[BOARD_SIZE];
extern const bitboard RAYS[8][BOARD_SIZE];
extern const bitboard singleTile[BOARD_SIZE];
extern const bitboard one;

void threatsSliding(gamestate* game, int position, int start, int end, int color, bitboard* result) {

	for (int i = start ; i < end ; i++) {
		bitboard onPath = game->bits & RAYS[i][position];
		int firstOnPath = -1;
		if (onPath) {
			// depending on the direction we must look for the most/least significant bit set
			switch (i) {
				case NORTH:
				case NORTH_WEST:
				case NORTH_EAST:
				case WEST:
					firstOnPath = msb(onPath);
					break;
				case SOUTH:
				case SOUTH_EAST:
				case SOUTH_WEST:
				case EAST:
					firstOnPath = lsb(onPath);
					break;
			}
			*result |= RAYS[i][position] & ~RAYS[i][firstOnPath];	// return the ray up to the first piece blocking it
			*result |= singleTile[firstOnPath];	// the first on path is accessible (it will be dismissed outside of this function if needed)
		}
		else {
			*result |= RAYS[i][position];
		}
	}
}

bitboard pawnMoves(gamestate* game, int position, int color) {

	bitboard access = pawnAttackMask[color][position];
	access |= pawnPositionMask[color][position] & ~game->bits;
	
	// The following code
	// ensure that there is nothing on path when moving 2 tiles
	bitboard onPath = game->bits & RAYS[color][position];
	int firstOnPath = -1;
	if (onPath) {
		if (color) {
			firstOnPath = lsb(onPath);
		} else {
			firstOnPath = msb(onPath);
		}
		access &= ~RAYS[color][firstOnPath];
	}

	return access;
}

bitboard kingMoves(gamestate* game, int position, int color) {
	// if the king cannont castle at all
	bitboard access = kingPositionMask[position];
	if (color == WHITE && position != 60) {
		return access;
	} else if (color == BLACK && position != 4) {
		return access;
	}
	
	// checking if there are pieces obstructing castlings
	bitboard onPath = game->bits & RAYS[2][position];
	int firstOnPath;
	if (onPath) {
		firstOnPath = msb(onPath);
		access &= ~RAYS[2][firstOnPath];
	}

	onPath = game->bits & RAYS[3][position];
	if (onPath) {
		firstOnPath = lsb(onPath);
		access &= ~RAYS[3][firstOnPath];
	}

	return access;
}

bitboard accessible(gamestate* game, int from, int color) {
	// return the tiles that can be accessed by the pieces at [from]
	// Note: for pawns, one must check for the presence of enemy
	// for the validity of the move
	bitboard access=0;
	int index = game->board[from];

	switch (game->pieces[index].type) {
		case PAWN:
			access = pawnMoves(game, from, color);
			break;
		case KING:
			access = kingMoves(game, from, color);
			break;
		case KNIGHT:
			access = knightAttackMask[from];
			break;
		case QUEEN:
			threatsSliding(game, from, 0, 8, color, &access);
			break;
		case ROOK:
			threatsSliding(game, from, 0, 4, color, &access);
			break;
		case BISHOP:
			threatsSliding(game, from, 4, 8, color, &access);
			break;
		default:
			access = 0;
	}
	access &= ~game->byColor[color];
	return access;
}

int threatened2(gamestate *game, int target, bitboard* result) {
	int oppCol = game->turn ? WHITE : BLACK;
	int off = oppCol * NB_PIECES / 2;
	int pos;

	bitboard threats = 0;

	for(int i = off ; i < off + NB_PIECES / 2 && (pos = game->pieces[i].position) != -1 ; i++) {
		if (game->board[pos] != i){	// if the index is invalid or if the piece was captured
			continue;
		} 
		else if (game->pieces[i].type == PAWN) {
			threats |= pawnAttackMask[oppCol][pos];
		} else if (game->pieces[i].type == KING) {
			threats |= kingAttackMask[pos];
		} else {
			threats |= accessible(game, pos, oppCol);
		}
	}
	if (result != NULL) {
		*result = threats;
	}
	return getAtIndex(threats, target);
}

int createAllMoves2(gamestate* game, moveList* list) {
	int size = 0;
	int offset = (NB_PIECES / 2)*game->turn;
	int position = -1;
	bitboard destination = 0;

	for (int index = offset ; index < NB_PIECES/2+offset && (position = game->pieces[index].position) != -1  ; index++) {
		if (game->board[position] != index) {
			continue;	// the piece was captured
		}
		destination = accessible(game, position, game->turn);
		for (int i = 0; i < BOARD_SIZE; i++) {
			if (destination & one) {
				moveList* m = (moveList*) malloc(sizeof(moveList));
				m->start = list->start;
				m->end = list->end;
				m->next = list->next;
				list->start = position;
				list->end = i;
				list->next = m;
				size++;
			}
			destination = destination >> 1;
		}
	}
	return size;
}
bitboard getPinned(gamestate game, int turn) {
	// pieces can be pinned only by rook, bishop, or queens
	bitboard result = 0;
	int oppCol = turn == WHITE;
	
	for (int i = 0; i < 4 ; i++) {	// rook and queen
		bitboard threats = RAYS[i][game.kingsIndex[turn]] & game.byColor[oppCol];
		threats &= (game.byType[QUEEN] | game.byType[ROOK]);
		if (threats != 0) {	// there are threats to the king
			int firstOnPath;
			if (i < 2) {
				firstOnPath = msb(threats);
			} else {
				firstOnPath = lsb(threats);
			}
			bitboard candidates = RAYS[i][game.kingsIndex[turn]] & game.bits & ~RAYS[i][firstOnPath];

			if (lsb(candidates) == msb(candidates)) {	// only one piece on ray
				result |= candidates;
			}
		}

	}
	for (int i = 4; i < 8 ; i++) {	// bishop and queen
		bitboard threats = RAYS[i][game.kingsIndex[turn]] & game.byColor[oppCol];
		threats &= (game.byType[QUEEN] | game.byType[BISHOP]);
		if (threats != 0) {	// there are threats to the king
			int firstOnPath;
			if (i < 6) {
				firstOnPath = msb(threats);
			} else {
				firstOnPath = lsb(threats);
			}
			bitboard candidates = RAYS[i][game.kingsIndex[turn]] & game.bits & ~RAYS[i][firstOnPath];

			if (lsb(candidates) == msb(candidates)) {	// only one piece on ray
				result |= candidates;
			}
		}
	}
	return result;
}

void freeMoveList(moveList* list) {
	if (list == NULL) {
		return;
	}
	freeMoveList(list->next);
	free(list);
}

int depthSearch(gamestate game, int depth) {
	const char promType[] = "qbnr";
	extern const int DEPTH;

	if (depth <= 0) {
		return 1;
	}
	int totalMoves = 0;
	moveList* list = (moveList*) malloc(sizeof(moveList));
	list->next = NULL;
	list->start = -1;
	list->end = -1;
	moveList* iter = list;
	int promotion = 0;
	int tmp = 0;
	gamestate newGame;
	
	createAllMoves2(&game, list); // might create illegal moves
	// those illegal moves will be ignored when actually doing the move
	bitboard pinned = getPinned(game, game.turn);

	while(list->start != -1) {
		newGame = game;
		if (doMove(iter->start, iter->end, &newGame, 0, &promotion, pinned)) {
			if (promotion) {
				for (int j = 1; j < 5; j++) {
					promote(&newGame, iter->end, j);
					tmp = depthSearch(newGame, depth-1);
					if (depth == DEPTH) {
						printMove(iter->start, iter->end, promType[j-1], tmp);
					}
					totalMoves += tmp;
				}
			} else {
				tmp = depthSearch(newGame, depth-1);
				totalMoves += tmp;
				if (depth == DEPTH) {
					printMove(iter->start, iter->end, 0, tmp);
				}
			}
		}
		iter = iter->next;
		free(list);
		list = iter;
	}
	return totalMoves;
}
		
int isInBound(int x, int y) {
	return x >= 0 && x < BOARD_WIDTH && y >= 0 && y < BOARD_HEIGHT;
}

void printMove(int start, int end, char prom, int nb) {
	char s[4];
	char e[4];
	indexToName(start, s);
	indexToName(end, e);
	if (prom) {
		printf("%s%s%c: %d\n", s, e, prom, nb);
	} else {
		printf("%s%s : %d\n", s, e, nb);
	}
}

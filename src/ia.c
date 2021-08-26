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

bitboard threatsSliding(gamestate* game, int position, int start, int end, int color) {
	int directions[] = {NORTH, SOUTH, WEST, EAST, NORTH_WEST, NORTH_EAST, SOUTH_WEST, SOUTH_EAST};
	extern const bitboard RAYS[8][BOARD_SIZE];
	bitboard result = 0;

	for (int i = start ; i < end ; i++) {
		bitboard onPath = game->bits & RAYS[directions[i]][position];
		int firstOnPath = -1;
		if (onPath) {
			// depending on the direction we must look for the most/least significant bit set
			switch (directions[i]) {
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
			result |= RAYS[directions[i]][position] & ~RAYS[directions[i]][firstOnPath];	// return the ray up to the first piece blocking it
			result |= 1 << firstOnPath;	// the first piece on path is also accessible
		}
		else {
			result |= RAYS[directions[i]][position];
		}
	}
	return result;
}

bitboard pawnMoves(gamestate* game, int position, int color) {
	int oppCol = game->turn ? WHITE : BLACK;
	bitboard access = pawnAttackMask[color][position];
	access &= game->byColor[oppCol];
	
	bitboard onPath = game->bits & RAYS[color][position];
	bitboard allowed = pawnPositionMask[color][position] & ~game->bits;
	int firstOnPath = -1;
	if (onPath) {
		if (color) {
			firstOnPath = lsb(onPath);
		} else {
			firstOnPath = msb(onPath);
		}
		access |= allowed & ~RAYS[color][firstOnPath];
	} else {
		access |= allowed;
	}

	return access;
}

bitboard kingMoves(gamestate* game, int position, int color) {
	if (color == WHITE && position != 60) {
		return kingPositionMask[position];
	} else if (color == BLACK && position != 4) {
		return kingPositionMask[position];
	}
	bitboard access = kingPositionMask[position];
	
	bitboard onPath = game->bits & RAYS[2][position];
	int firstOnPath = -1;
	if (onPath) {
		firstOnPath = msb(onPath);
		access &= ~RAYS[2][firstOnPath];
	}

	onPath = game->bits & RAYS[3][position];
	firstOnPath = -1;
	if (onPath) {
		firstOnPath = lsb(onPath);
		access &= ~RAYS[3][firstOnPath];
	}

	return access;
}

bitboard accessible(gamestate* game, int from, int color) {
	// return the tiles that can be *attacked* from the given position ([from])
	// so to get the accesible tiles for the pawns, the user must OR the result
	// with the proper mask
	// TODO: add en passant spot ?
	bitboard access = 0;
	int index = game->board[from];

	switch (game->pieces[index].type) {
		case PAWN:
			access = pawnMoves(game, from, color);
			access |= pawnAttackMask[color][game->pieces[index].position];
			break;
		case KING:
			//access |= kingAttackMask[from];
			access = kingMoves(game, from, color);
			break;
		case KNIGHT:
			access |= knightAttackMask[from];
			break;
		case QUEEN:
			access |= threatsSliding(game, from, 0, 8, color);
			break;
		case ROOK:
			access |= threatsSliding(game, from, 0, 4, color);
			printBits(access);
			break;
		case BISHOP:
			access |= threatsSliding(game, from, 4, 8, color);
			break;
	}
	access &= ~game->byColor[color];
	return access;
}

int threatened2(gamestate *game, int target, bitboard* result) {
	int oppCol = game->turn ? WHITE : BLACK;
	int off = oppCol * NB_PIECES / 2;

	bitboard threats = 0;

	for(int i = off ; i < off + NB_PIECES / 2 ; i++) {
		int pos = game->pieces[i].position;
		if (pos == -1 || pos > BOARD_SIZE
				|| game->board[pos] != i){	// if the index is invalid or if the piece was captured
			continue;
		} else if (game->pieces[i].type == PAWN) {
			threats |= pawnAttackMask[oppCol][pos];
		} else {
			threats |= accessible(game, pos, oppCol);
		}
	}
	threats &= ~game->byColor[oppCol];	// avoid allied pieces
	if (result != NULL) {
		*result = threats;
	}
	return getAtIndex(threats, target);
}



int pawnOff[] = {-8, -16, -7, -9, 8, 16, 7, 9};
int knightOff[] = {-6, -10, -15, -17, 6, 10, 15, 17};
int offsets[] = {-1, 1, -8, 8, -9, 9, -7, 7};
int kingOff[] = {-1, 1, -8, 8, -9, 9, -7, 7, 2, -2};

int simShortRange(int result[], gamestate* game, int index) {
	int *offList;
	int beg = 0;
	int end = 8;
	int endPiece = game->board[index];
	int size = 0;
	switch(game->pieces[endPiece].type) {
		case PAWN:
			offList = pawnOff;
			beg =    4 * game->pieces[endPiece].color;
			end = 4 + 4 * game->pieces[endPiece].color;
			break;
		case KING:
			offList = kingOff;
			end = 10;
			break;
		case KNIGHT:
			offList = knightOff;
			break;
	}
	for (int i = beg ; i < end; i++) {
		int newIndex = index + offList[i];
		if (newIndex >= 0 && newIndex < BOARD_SIZE) {
			if (doMove(index, newIndex, game, 1)) {
					result[size++] = newIndex;
			}
		}
	}
	return size;
}

int simLongRange(int result[], gamestate* game, int index) {
	int beg = 0;
	int end = 8;
	int size = 0;
	int endPiece = game->board[index];
	int x = index % BOARD_WIDTH;
	int y = index / BOARD_WIDTH;

	// Avoid repetition of indexes
	int distLeft = x;	// distance to the left of the board
	int distRight = BOARD_WIDTH - x -1;
	int distUp = y;
	int distDown = BOARD_HEIGHT - y -1;
	int distDiag1 = min(distLeft, distUp);
	int distDiag2 = min(distRight, distDown);
	int distDiag3 = min(distRight, distUp);
	int distDiag4 = min(distLeft, distDown);
	int dist[] = {distLeft, distRight, distUp, distDown, 
		distDiag1, distDiag2, distDiag3, distDiag4};

	switch(game->pieces[endPiece].type) {
		case ROOK:
			end = 4;
			break;
		case BISHOP:
			beg = 4;
			break;
	}

	for ( int i = beg; i < end; i++) {
		int newIndex = index + offsets[i];
		for (int j = 0 ; j < dist[i] && newIndex >= 0 && newIndex < BOARD_SIZE ; j++, newIndex += offsets[i]) {
			if (doMove(index, newIndex, game, 1)) {
				result[size++] = newIndex;
			}
		}
	}
	return size;
}

int createAllMoves(gamestate* game, moveList* list) {

	int size = 0;
	int offset = (NB_PIECES / 2)*game->turn;

	for (int index = offset ; index < NB_PIECES/2+offset ; index++) {
		if (game->pieces[index].position == -1) {
			continue;
		}
		if (game->board[game->pieces[index].position] != index) {
			continue;	// the piece was captured
		}
		//if (game->pieces[index].color != game->turn) {
		//	continue;
		//}
		int result[64]={0};
		int nbMoves = 0;
		switch (game->pieces[index].type) {
			case PAWN:
			case KNIGHT:
			case KING:
				nbMoves = simShortRange(result, game, game->pieces[index].position);
				break;
			case QUEEN:
			case ROOK:
			case BISHOP:
				nbMoves = simLongRange(result, game, game->pieces[index].position);
				break;
			case NONE_P:
				continue;
				break;
		}
		for (int i = 0; i < nbMoves; i++) {
			moveList* m = (moveList*) malloc(sizeof(moveList*));
			m->start = list->start;
			m->end = list->end;
			m->next = list->next;
			list->start = game->pieces[index].position;
			list->end = result[i];
			list->next = m;
			size++;
		}
	}
	return size;
}

int createAllMoves2(gamestate* game, moveList* list) {
	int size = 0;
	int offset = (NB_PIECES / 2)*game->turn;
	int position = -1;
	bitboard destination = 0;

	for (int index = offset ; index < NB_PIECES/2+offset ; index++) {
		position = game->pieces[index].position;
		if (position == -1) {
			continue;	// there is no piece
		}
		if (!getAtIndex(game->bits, position)) {
			continue;	// the piece was captured
		}
		destination = accessible(game, game->pieces[index].position, game->turn);
		if (game->pieces[index].type == PAWN) {
			// accessible returns the tiles that can be attacked
			destination |= pawnPositionMask[game->turn][position];
			// might have illegal moves which will be filtered later
			// (on attack moves only because of en passant)
		}
		destination &= ~game->byColor[game->turn];	//avoid tiles occupied by allies
		for (int i = 0; i < BOARD_SIZE; i++) {
			if (!getAtIndex(destination, i)) {
				continue;
			}
			moveList* m = (moveList*) malloc(sizeof(moveList*));
			m->start = list->start;
			m->end = list->end;
			m->next = list->next;
			list->start = position;
			list->end = i;
			list->next = m;
			size++;
		}
	}
	return size;
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
	moveList* list = malloc(sizeof(moveList*));
	list->next = NULL;
	
	createAllMoves2(&game, list);

	moveList* iter = list;
	for (int i = 0 ; iter->next != NULL ; iter = iter->next, i++ ) {
		gamestate newGame = game;
		if (!doMove(iter->start, iter->end, &newGame, 0)) {
			continue;
		}
		int promotion = isPromotion(&newGame);
		int tmp = 0;
		if (promotion != -1) {
			for ( int j = 1; j < 5; j++) {
				promote(&newGame, promotion, j);
				tmp = depthSearch(newGame, depth-1);
				char letter = promType[j-1];
				if (depth == DEPTH) {
					printMove(iter->start, iter->end, letter, tmp);
				}
				totalMoves += tmp;
			}
			continue;
		}
		tmp = depthSearch(newGame, depth-1);
		if (depth == DEPTH) {
			printMove(iter->start, iter->end, 0, tmp);
		}
		totalMoves += tmp;
	}
	freeMoveList(list);
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

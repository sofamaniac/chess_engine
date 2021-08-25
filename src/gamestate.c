# include "ia.h"
# include "bitboard.h"
# include "gamestate.h"
# include "game.h"
# include "tools.h"

extern const tile emptyTile;
extern const bitboard pawnAttackMask[2][BOARD_SIZE];
extern const bitboard pawnPositionMask[2][BOARD_SIZE];
extern const bitboard kingAttackMask[BOARD_SIZE];
extern const bitboard kingPositionMask[BOARD_SIZE];
extern const bitboard knightAttackMask[BOARD_SIZE];
extern const bitboard RAYS[8][BOARD_SIZE];

void setBit(gamestate* game, int index) {
	setAtIndex(&game->bits, index);
	setAtIndex(&game->byColor[game->turn], index);
}
void resetBit(gamestate* game, int index) {
	resetAtIndex(&game->bits, index);
	resetAtIndex(&game->byColor[game->turn], index);
}
void move(gamestate* game, int start, int end) {
	// use propagate move on all gamestate structure members	
	// this function should be used to undo the move by inversing the order of start and end
	setAtIndex(&game->byColor[game->turn], end);
	resetAtIndex(&game->byColor[game->turn], start);
	resetAtIndex(&game->byColor[game->turn == WHITE], end);
	// game->bits = game->byColor[WHITE] | game->byColor[BLACK];
}
void undo(gamestate* game, int start, int end) {
	move(game, end, start);
}

int doMove(int start, int end, gamestate* game, int revert) {
	// if revert is set to 1, then the move is only simulated and not executed

	int endPiece = game->board[end];
	int startPiece = game->board[start];
	int enPassant = 0;
	int nextPassant = -1;
	int castling = -1;
	int pawnDir = game->turn ? 1 : -1;

	int kingIndex = game->kingsIndex[game->turn];
	int pseudolegal = 1;
	int success = 0;
	int old = game->board[end];
	int oppColor = game->turn == WHITE ? BLACK : WHITE;


	if ( start == end ) {
		return 0;
	} else if ( start < 0 || start > BOARD_SIZE
			|| end < 0 || end > BOARD_SIZE ) {
		return 0;
	} else if ( startPiece == NONE_P) {
		return 0;
	} else if (endPiece != NONE_P && game->pieces[endPiece].color == game->turn) {
		return 0;
	} else if (game->pieces[startPiece].color != game->turn) {
		return 0;
	}

	bitboard access = accessible(game, start, game->turn);
	if (game->pieces[startPiece].type == PAWN) {
		// check the en passant pawn position
		enPassant = (end == game->enPassantTarget);	// no need to check if the pawn is still there
		if (abs(end-start) == 16) {
			nextPassant = end - BOARD_WIDTH*pawnDir;
			access &= ~pawnAttackMask[game->turn][start];
			access &= ~game->byColor[oppColor];	// the destination should be empty, but one must check if there is nothing on the path
		} else if (abs(end-start) != 8) { // attempt a capture
			access &= game->byColor[oppColor]; // there must be an enemy at the end
		} else {
			access &= ~pawnAttackMask[game->turn][start];
			access &= ~game->byColor[oppColor]; // otherwise the destination should be empty
		}
	} else if (game->pieces[startPiece].type == KING) {
		kingIndex = end;
	}
	access &= ~game->byColor[game->turn];
	pseudolegal = getAtIndex(access, end);
	if(!pseudolegal && !enPassant) {
		return 0;
	}

	game->board[end] = game->board[start];
	game->board[start] = NONE_P;
	move(game, start, end);

	bitboard threats = 0;
	int nbThreats;
	if (enPassant) {
		int tmp = end - BOARD_WIDTH*pawnDir;
		int oldEnPassant = game->board[tmp];
		game->board[tmp] = NONE_P;

		resetAtIndex(&game->bits, tmp);
		resetAtIndex(&game->byColor[oppColor], tmp);
		nbThreats = threatened2(game, kingIndex, &threats);
		if (nbThreats || revert) {
			game->board[tmp] = oldEnPassant;
			setAtIndex(&game->bits, tmp);
			setAtIndex(&game->byColor[oppColor], tmp);
			if(nbThreats) {
				goto reset;
			}
		}
	}else {
		nbThreats = threatened2(game, kingIndex, &threats);
		if (start == nameToIndex("b5")) {
			printBits(threats);
		}
		if (nbThreats > 0) {
			goto reset;
		}
	}
	if (game->pieces[old].type == ROOK) {
		castling = 8 + game->pieces[old].color;
	}
	// the condition is correct ? I think not
	if (castling != -1 && castling < 4) {
		if ( castling % 2 == 0){ // king's side castling
			//if (threatened(game, start, threats) || threatened(game, start+1, threats)) {
			if (getAtIndex(threats, start) || getAtIndex(threats, start+1)) {
				goto reset;
			} else if (!revert) {
				game->board[start+1] = game->board[start+3];
				game->pieces[game->board[start+3]].position = start+1;
				game->board[start+3] = NONE_P;
				move(game, start+3, start+1);
			}
		} else { // Queen's side castling
			// if (threatened(game, start, threats) || threatened(game, start-1, threats)) {
			if (getAtIndex(threats, start) || getAtIndex(threats, start-1)) {
				goto reset;
			} else if (!revert){
				game->board[start-1] = game->board[start-4];
				game->pieces[game->board[start-4]].position = start-1;
				game->board[start-4] = NONE_P;
				move(game, start-4, start-1);
			}
		}
	}
	success = 1;
	if (revert) {
		goto reset;
	}

	game->pieces[startPiece].position = end;
	game->board[end] = startPiece;
	game->enPassantTarget = nextPassant;
	updateCastling(game, castling, start, end);
	game->kingsIndex[game->turn] = kingIndex;
	move(game, start, end);
	game->turn = oppColor;
	game->bits = game->byColor[WHITE] | game->byColor[BLACK];
	goto end;

reset:
	game->board[start] = game->board[end];
	undo(game, start, end);
	game->board[end] = old;
	if (old != NONE_P) {
		setAtIndex(&game->byColor[oppColor], end);
	}
	game->bits = game->byColor[WHITE] | game->byColor[BLACK];
end:
	return success;

}

void updateCastling(gamestate* game, int castling, int start, int end) {

	int endPiece = game->board[end];
	
	if (castling == -1) {
		return;
	} else if (castling < 4) {	// the king castled
		int color = game->pieces[endPiece].color;
		game->castling[2*color] = 0;
		game->castling[2*color+1] = 0;
	} else if (castling < 6) {	// the king moved without castling
		int color = castling % 2;
		game->castling[2*color] = 0;
		game->castling[2*color+1] = 0;
	} else {					// a rook moved or was captured
		int ref = castling < 8 ? start : end;
		if (ref == 63) {
			game->castling[0] = 0;
		} else if (ref == 56) {
			game->castling[1] = 0;
		} else if ( ref == 7) {
			game->castling[2] = 0;
		} else if (ref == 0) {
			game->castling[3] = 0;
		}
	}
}

int isPromotion(gamestate* game) {
	for(int i = 0; i < BOARD_WIDTH; i ++) {
		int up = game->board[i];
		int down = game->board[ (BOARD_HEIGHT-1) * BOARD_WIDTH + i ];
		if (game->pieces[up].type == PAWN) {
			return i;
		} else if (game->pieces[down].type == PAWN) {
			return (BOARD_HEIGHT-1) * BOARD_WIDTH + i;
		}
	}
	return -1;
}

void promote(gamestate* game, int index, int target) {
	int endPiece = game->board[index];
	game->pieces[endPiece].type = target;
}

int checkObstruction(gamestate* game, int startX, int startY, int endX, int endY, int dx, int dy) {
	int x, y;
	for (y = startY + dy, x = startX + dx; y != endY || x != endX; y += dy, x += dx) {
		if (game->board[index(x, y)] != NONE_P) {
			return 0;
		}
	}
	return 1;
}

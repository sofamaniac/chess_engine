# ifndef GAMESTATE_H
# define GAMESTATE_H

# define BOARD_HEIGHT 8
# define BOARD_WIDTH 8
# define NB_PIECES 32
# define BOARD_SIZE (BOARD_HEIGHT * BOARD_HEIGHT)
# define index(X, Y) ((X) + BOARD_WIDTH * (Y))
# include "bitboard.h"

enum colors {
	WHITE,
	BLACK,
};

enum pieces {
	KING,
	QUEEN,
	BISHOP,
	KNIGHT,
	ROOK,
	PAWN,
	NONE_P=NB_PIECES	// index in the gamestate struct
};

typedef struct tile {
	unsigned int color;
	unsigned int type;
	unsigned int position;
} tile;

typedef struct gamestate {
	int board[BOARD_SIZE];	// chess board
	int castling[4];		// K Q, k, q
	int enPassantTarget;	// target for en passant
	int turn;				// who is playing
	int halfmoves;			// time since last capture
	int clock;				// total number of turns
	int kingsIndex[2];		// position of both kings
	tile pieces[NB_PIECES+1];		// 0-15 for white ; 16-31 for black ; 32 for empty tile
	bitboard bits;
	bitboard byColor[2];	// bitboard by color of pieces
	bitboard byType[6];		// bitboard by type of pieces
} gamestate;


int doMove(int start, int end, gamestate* game, int revert, int* isPromotion, bitboard pinned);
void updateCastling(gamestate* game, int castling, int start, int end);
int checkObstruction(gamestate* game, int startX, int startY, int endX, int endY, int dx, int dy);
int isPromotion(gamestate* game);
void promote(gamestate* game, int index, int target);
# endif

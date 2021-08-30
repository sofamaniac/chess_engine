# include "bitboard.h"
# include "gamestate.h"
# include "tools.h"
# include <stdlib.h>
# include <stdio.h>

bitboard RAYS[8][BOARD_SIZE]; // direction, starting position
bitboard kingAttackMask[BOARD_SIZE];
bitboard knightAttackMask[BOARD_SIZE];
bitboard pawnAttackMask[2][BOARD_SIZE]; // Whites and Blacks

bitboard knightPositionMask[BOARD_SIZE];
bitboard kingPositionMask[BOARD_SIZE];
bitboard pawnPositionMask[2][BOARD_SIZE]; // Whites and Blacks

// we define a constant to avoid int overflow
const bitboard one = 1;

// we define numbers with only one bit set
bitboard singleTile[BOARD_SIZE];

__attribute__((always_inline)) void setAtIndex(bitboard* b, unsigned int index) {
	*b |= singleTile[index];
}
__attribute__((always_inline)) void resetAtIndex(bitboard* b, unsigned int index) {
	*b &= ~(singleTile[index]);
}
void togglingIndex(bitboard* b, unsigned int index) {
	*b ^= singleTile[index];
}
__attribute__((always_inline)) int getAtIndex(bitboard b, unsigned int index) {
	return (b >> index) & one;
}

void createRaysLines() {

	for (unsigned int i = 0; i < BOARD_SIZE ; i++) {
		RAYS[NORTH][i] = 0;
		RAYS[SOUTH][i] = 0;
		RAYS[WEST][i] = 0;
		RAYS[EAST][i] = 0;
		unsigned int x = i % BOARD_WIDTH;
		unsigned int y = i / BOARD_HEIGHT;

		for (int y2 = 0; y2 < y; y2++) {
			RAYS[NORTH][i] |= one << index(x, y2);
		}
		for (int y2 = y+1; y2 < BOARD_HEIGHT; y2++) {
			RAYS[SOUTH][i] |= one << index(x, y2);
		}
		for (int x2 = 0; x2 < x; x2++) {
			RAYS[WEST][i] |= one << index(x2, y);
		}
		for (int x2 = x+1; x2 < BOARD_WIDTH; x2++) {
			RAYS[EAST][i] |= one << index(x2, y);
		}
	}
}

void createRaysDiags() {

	for ( unsigned int i = 0 ; i < BOARD_SIZE ; i++) {
		RAYS[NORTH_WEST][i] = 0;
		RAYS[NORTH_EAST][i] = 0;
		RAYS[SOUTH_WEST][i] = 0;
		RAYS[SOUTH_EAST][i] = 0;
		int x = i % BOARD_WIDTH;
		int y = i / BOARD_WIDTH;

		int dx, dy;
		for (dx = -1, dy = -1; x + dx >= 0 && y + dy >= 0; dx--, dy--) {
			RAYS[NORTH_WEST][i] |= one << index(x+dx, y+dy);
		}
		for (dx = 1, dy = -1; x + dx < BOARD_WIDTH && y + dy >= 0; dx++, dy--) {
			RAYS[NORTH_EAST][i] |= one << index(x+dx, y+dy);
		}
		for (dx = -1, dy = 1; x + dx >= 0 && y + dy < BOARD_WIDTH; dx--, dy++) {
			RAYS[SOUTH_WEST][i] |= one << index(x+dx, y+dy);
		}
		for (dx = 1, dy = 1; x + dx < BOARD_WIDTH && y + dy < BOARD_HEIGHT; dx++, dy++) {
			RAYS[SOUTH_EAST][i] |= one << index(x+dx, y+dy);
		}
	}
}

void createKingAttackMask() {

	for (unsigned int i = 0 ; i < BOARD_SIZE ; i++) {
		kingAttackMask[i] = 0;
		unsigned int x = i % BOARD_WIDTH;
		unsigned int y = i / BOARD_WIDTH;

		for (int dx = -1 ; dx < 2 ; dx++) {
			for (int dy = -1 ; dy < 2 ; dy++) {
				if (dx == 0 && dy == 0) {
					continue;
				}
				if (x+dx >= 0 && x + dx < BOARD_WIDTH
						&& y + dy >= 0 && y + dy < BOARD_HEIGHT) {
					kingAttackMask[i] |= one << index(x+dx, y+dy);
				}
			}
		}
	}
}

void createPawnAttackMask() {
	for (unsigned int c = 0; c < 2; c++) {
		int dy = -1 + 2*c;
		for (unsigned int i = 0; i < BOARD_SIZE; i++) {
			pawnAttackMask[c][i] = 0;
			int x = i % BOARD_WIDTH;
			int y  = i / BOARD_WIDTH;
			for (int dx = -1; dx < 2 ; dx+=2) {
				if (x+dx >= 0 && x+dx < BOARD_WIDTH
						&& y + dy >= 0 && y + dy < BOARD_HEIGHT) {
					pawnAttackMask[c][i] |= one << index(x+dx, y+dy);
				}
			}
		}
	}
}

void createKnightAttackMask() {

	int delta[] = {-2, -1, 2, 1};

	for (unsigned int i = 0; i < BOARD_SIZE; i++) {
		knightAttackMask[i] = 0;
		int x = i % BOARD_WIDTH;
		int y  = i / BOARD_WIDTH;
		for (int dy = 0 ; dy < 4; dy++) {
			for (int dx = 0; dx < 4; dx++) {
				if (dx % 2 == dy % 2) {
					continue;
				}
				if (x+delta[dx] >= 0 && x+delta[dx] < BOARD_WIDTH
						&& y + delta[dy] >= 0 && y + delta[dy] < BOARD_HEIGHT) {
					knightAttackMask[i] |= one << index(x+delta[dx], y+delta[dy]);
				}
			}
		}
	}
}


void createAttackMask() {
	createRaysLines();
	createRaysDiags();
	createKingAttackMask();
	createPawnAttackMask();
	createKnightAttackMask();

}

void createPawnPositionMask() {
	for (unsigned int c = 0; c < 2; c++) {
		int dy = -1 + 2*c;
		for (unsigned int i = 0; i < BOARD_SIZE; i++) {
			pawnPositionMask[c][i] = 0;
			int x = i % BOARD_WIDTH;
			int y  = i / BOARD_WIDTH;
			if (y + dy >= 0 && y + dy < BOARD_HEIGHT) {
				pawnPositionMask[c][i] |= one << index(x, y+dy);
			}
		}
		dy = 2*dy;
		int startRow = c ? 1 : 6;
		for (unsigned int x = 0; x < BOARD_WIDTH; x++) {
			pawnPositionMask[c][index(x, startRow)] |= one << index(x, startRow+dy);
		}

	}
}

void createKingPositionMask() {
	for (unsigned int i = 0; i < BOARD_SIZE; i++) {
		kingPositionMask[i] = kingAttackMask[i];
	}
	kingPositionMask[nameToIndex("e1")] |= one << nameToIndex("c1");
	kingPositionMask[nameToIndex("e1")] |= one << nameToIndex("g1");

	kingPositionMask[nameToIndex("e8")] |= one << nameToIndex("c8");
	kingPositionMask[nameToIndex("e8")] |= one << nameToIndex("g8");

}
void createKnightPositionMask() {
	for (unsigned int i = 0; i < BOARD_SIZE; i++) {
		knightPositionMask[i] = knightAttackMask[i];
	}
}

void createPositionMask() {
	createKingPositionMask();
	createPawnPositionMask();
	createKnightPositionMask();
}
void createSingleTile() {
	for (int i = 0 ; i < BOARD_SIZE ; i++ ) {
		singleTile[i] = one << i;
	}
}

void initMask() {
	createSingleTile();
	createAttackMask();
	createPositionMask();
}
const int index64[64] = {
    0, 47,  1, 56, 48, 27,  2, 60,
   57, 49, 41, 37, 28, 16,  3, 61,
   54, 58, 35, 52, 50, 42, 21, 44,
   38, 32, 29, 23, 17, 11,  4, 62,
   46, 55, 26, 59, 40, 36, 15, 53,
   34, 51, 20, 43, 31, 22, 10, 45,
   25, 39, 14, 33, 19, 30,  9, 24,
   13, 18,  8, 12,  7,  6,  5, 63
};

/**
 * bitScanReverse
 * @authors Kim Walisch, Mark Dickinson
 * @param bb bitboard to scan
 * @precondition bb != 0
 * @return index (0..63) of most significant one bit
 */
// wanted to use __builtin_clz, but it's limited to 32 bits
__attribute__((always_inline)) int bitScanReverse(bitboard bb) {
   const bitboard debruijn64 = (bitboard) 0x03f79d71b4cb0a89;
   bb |= bb >> 1; 
   bb |= bb >> 2;
   bb |= bb >> 4;
   bb |= bb >> 8;
   bb |= bb >> 16;
   bb |= bb >> 32;
   return index64[(bb * debruijn64) >> 58];
}

/**
 * bitScanForward
 * @author Kim Walisch (2012)
 * @param bb bitboard to scan
 * @precondition bb != 0
 * @return index (0..63) of least significant one bit
 */
__attribute__((always_inline)) int bitScanForward(bitboard bb) {
   const bitboard debruijn64 = (bitboard) 0x03f79d71b4cb0a89;
   return index64[((bb ^ (bb-1)) * debruijn64) >> 58];
}

# ifndef BITBOARD_H
# define BITBOARD_H
# include <stdint.h>

typedef uint_fast64_t bitboard;

# define msb bitScanReverse
# define lsb bitScanForward

enum directions {
	NORTH,
	SOUTH,
	WEST,
	EAST,
	NORTH_WEST,
	NORTH_EAST,
	SOUTH_WEST,
	SOUTH_EAST
};

void setAtIndex(bitboard* b, unsigned int index);
void resetAtIndex(bitboard* b, unsigned int index);
void togglingIndex(bitboard* b, unsigned int index);
int getAtIndex(bitboard b, unsigned int index);
void createAttackMask();
void createPositionMask();
void createSingleTile();
void initMask();
int bitScanReverse(bitboard b);
int bitScanForward(bitboard bb);

# endif

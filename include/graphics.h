# ifndef GRAPHICS_H
# define GRAPHICS_H

# include <SDL2/SDL.h>
# include "game.h"

# define min(X, Y) ((X) < (Y) ? (X) : (Y))

# define HEIGHT 600
# define WIDTH 600
# define IMAGE_WIDTH 428
# define IMAGE_HEIGHT 428


typedef struct color {
	unsigned int r:8;
	unsigned int g:8;
	unsigned int b:8;
	unsigned int a:8;
} color;

int init(SDL_Window** window, SDL_Renderer** renderer);
void quit(SDL_Window* window, SDL_Renderer* renderer);
void drawSquare(SDL_Renderer* renderer, int x, int y, int w, int h, color c);
void drawThreats(SDL_Renderer* renderer, int board[], int board_width, int board_height);
void drawBoard(SDL_Renderer* renderer, int board_width, int board_height, tile board[]);
void drawPiece(SDL_Renderer* renderer, int x, int y, int type);
int loadImage(SDL_Renderer* renderer);
int pieceToInt(tile t);
int tileSize(SDL_Renderer* renderer);

# endif

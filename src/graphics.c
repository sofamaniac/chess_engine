# include <SDL2/SDL.h>
# include <SDL2/SDL_image.h>

# include "graphics.h"
# include "game.h"

SDL_Texture* chess_pieces=NULL;

const SDL_Rect WKing   = { .x=0*IMAGE_WIDTH, .y=0, .w=IMAGE_WIDTH, .h=IMAGE_HEIGHT };
const SDL_Rect WQueen  = { .x=1*IMAGE_WIDTH, .y=0, .w=IMAGE_WIDTH, .h=IMAGE_HEIGHT };
const SDL_Rect WBish   = { .x=2*IMAGE_WIDTH, .y=0, .w=IMAGE_WIDTH, .h=IMAGE_HEIGHT };
const SDL_Rect WKnight = { .x=3*IMAGE_WIDTH, .y=0, .w=IMAGE_WIDTH, .h=IMAGE_HEIGHT };
const SDL_Rect WRook   = { .x=4*IMAGE_WIDTH, .y=0, .w=IMAGE_WIDTH, .h=IMAGE_HEIGHT };
const SDL_Rect WPawn   = { .x=5*IMAGE_WIDTH, .y=0, .w=IMAGE_WIDTH, .h=IMAGE_HEIGHT };

const SDL_Rect BKing   = { .x=0*IMAGE_WIDTH, .y=IMAGE_HEIGHT, .w=IMAGE_WIDTH, .h=IMAGE_HEIGHT };
const SDL_Rect BQueen  = { .x=1*IMAGE_WIDTH, .y=IMAGE_HEIGHT, .w=IMAGE_WIDTH, .h=IMAGE_HEIGHT };
const SDL_Rect BBish   = { .x=2*IMAGE_WIDTH, .y=IMAGE_HEIGHT, .w=IMAGE_WIDTH, .h=IMAGE_HEIGHT };
const SDL_Rect BKnight = { .x=3*IMAGE_WIDTH, .y=IMAGE_HEIGHT, .w=IMAGE_WIDTH, .h=IMAGE_HEIGHT };
const SDL_Rect BRook   = { .x=4*IMAGE_WIDTH, .y=IMAGE_HEIGHT, .w=IMAGE_WIDTH, .h=IMAGE_HEIGHT };
const SDL_Rect BPawn   = { .x=5*IMAGE_WIDTH, .y=IMAGE_HEIGHT, .w=IMAGE_WIDTH, .h=IMAGE_HEIGHT };
SDL_Rect pieces[12];

int init(SDL_Window** window, SDL_Renderer** renderer)
{
	int exit_code = EXIT_SUCCESS;
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
		exit_code = EXIT_FAILURE;
	}
	
	// creation of the window
	*window = SDL_CreateWindow( "tetris", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);

	if ( window == NULL) {
		exit_code = EXIT_FAILURE;
	}

	// creation of the renderer
	// The second parameter is used to determine the graphical pilot to load
	// and therefore depends on the flags used, a value of -1 let SDL choose the right value
	*renderer = SDL_CreateRenderer( *window, -1 ,SDL_RENDERER_ACCELERATED);
	if (renderer == NULL) {
		exit_code = EXIT_FAILURE;
	}
	if( loadImage(*renderer) != 0 ){
		exit_code = EXIT_FAILURE;
	}
	if (exit_code != EXIT_SUCCESS){
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[debug] %s", SDL_GetError());
	}

	pieces[0] = WKing;
	pieces[1] = WQueen;
	pieces[2] = WBish;
	pieces[3] = WKnight;
	pieces[4] = WRook;
	pieces[5] = WPawn;

	pieces[6] = BKing;
	pieces[7] = BQueen;
	pieces[8] = BBish;
	pieces[9] = BKnight;
	pieces[10] = BRook;
	pieces[11] = BPawn;
	return exit_code;
}

void quit(SDL_Window* window, SDL_Renderer* renderer)
{
	//SDL_DestroyTexture(chess_pieces);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void drawSquare(SDL_Renderer* renderer, int x, int y, int w, int h, color c) {
	// draw a rectangle at (x, y) of dimension wxh and filled with color c
	SDL_Rect rect = { x, y, w, h };
	SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
	SDL_RenderFillRect(renderer, &rect);
}

void drawThreats(SDL_Renderer* renderer, int board[], int board_width, int board_height) {
	color red = {225, 25, 25,};
	unsigned int tile_size = tileSize(renderer);

	for (int j = 0; j < board_height; j++) {
		for (int i = 0; i < board_width; i++) {
			if ( board[index(i, j)] ) {
				drawSquare(renderer, i*tile_size, j*tile_size, tile_size, tile_size, red);
			}
		}
	}
}

void drawBoard(SDL_Renderer* renderer, int board_width, int board_height, tile board[]) {

	color white = { 200, 200, 200, 255 };
	color black = { 70,   70,  70, 255 };

	unsigned int tile_size = tileSize(renderer);

	for (int j = 0; j < board_height; j++) {
		for (int i = 0; i < board_width; i++) {
			if ( (i+j) % 2 == 1 ) {
				drawSquare(renderer, i*tile_size, j*tile_size, tile_size, tile_size, black);
			}
			else {
				drawSquare(renderer, i*tile_size, j*tile_size, tile_size, tile_size, white);
			}
			tile current_tile = board[index(i, j)];
			if (current_tile.type != NONE_P) {
				drawPiece(renderer, i*tile_size, j*tile_size, pieceToInt(current_tile));
			}
		}
	}
}

int loadImage(SDL_Renderer* renderer) {
	chess_pieces = IMG_LoadTexture(renderer, "../Chess_Pieces_Sprite.png");
	if (chess_pieces == NULL)
		return -1;
	return 0;
}

void drawPiece(SDL_Renderer* renderer, int x, int y, int type) {
	int tile_size = tileSize(renderer);
	SDL_Rect dest;
	dest.x=x;
	dest.y=y;
	dest.w=tile_size;
	dest.h=tile_size;
	if (SDL_RenderCopy(renderer, chess_pieces, &(pieces[type]), &dest) != 0) {
		printf("%s\n", SDL_GetError());
	}
}

int pieceToInt(tile t) {
	return 6*t.color + t.type;
}

int tileSize(SDL_Renderer* renderer) {
	int screen_w, screen_h;

	if (SDL_GetRendererOutputSize(renderer, &screen_w, &screen_h) != 0 ) {
		return -1;
	}
	unsigned int small_side = min(screen_w, screen_h);	// get the smallest dimensions of the two

	unsigned int tile_size = min(small_side / BOARD_WIDTH, small_side / BOARD_HEIGHT);

	return tile_size;
}

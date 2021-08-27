# include <ctype.h>
# include <SDL2/SDL.h>
# include "game.h"
# include "graphics.h"
# include "ia.h"
# include "tools.h"
# include "gamestate.h"
# include <pthread.h>

const tile emptyTile = { .color=WHITE, .type=NONE_P };
int DEPTH = 6;
SDL_Renderer* renderer;


int cli(void) {
	char command[1000]="";
	gamestate game;
	int run = 1;
	createAttackMask();
	createPositionMask();
	while(run) {
		memset(command, 0, sizeof(command));	// reset command to empty string
		scanf("%[^\n]", command);
		fgetc( stdin );	// to delete the '\n' character from the buffer
		if (startWith("quit", command)) {
			run = 0;
		} else if (startWith("position", command)) {
			parseConfig(command, &game);
		} else if (startWith("go perft", command)) {
			parseSearch(command, &game);
		} else if (startWith("d", command)) {
			printBoard(game);
		} else if (startWith("isready", command)) {
			printf("readyok");
		} else if (startWith("threats", command)) {
			//int threats[BOARD_SIZE];
			//int nbThreats = threatened(&game, nameToIndex(command + 8), threats);
			//printThreats2(threats, nbThreats);
			bitboard threats = 0;
			threatened2(&game, nameToIndex(command + 8), &threats);
			printBits(threats);
		} else if (startWith("bitboard", command)) {
			printBits(game.bits);
		} else if (startWith("accessible", command)) {
			int index = nameToIndex(command+11);
			printBits(accessible(&game, index, game.board[index] >= 16));
		} else {
			printf("Unknown command %s\n", command);
		}
		printf("\n");	// used to flush the ouput
	}
	return 1;
}



int mainLoop(SDL_Window* window, SDL_Renderer* r) {
	// function handling the game loop
	
	renderer = r;

	int isOpen = 1;
	SDL_Event event;

	int graphics_tick = 0;

	int selectIndex = -1;
	tile selectPiece = { .color=WHITE, .type=NONE_P };
	
	gamestate game;
	//loadConfiguration("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", &game);
	loadConfiguration("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ", &game);
	
	printf("nb moves : %d\n", depthSearch(game, DEPTH)); 

	
	/*
	int result[64];
	int size = threatened(game, nameToIndex("h3"), result);
	doMove(nameToIndex("h5"), nameToIndex("h1"), &game);
	printThreats2(result, size);
	*/

	while(isOpen) {
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
					isOpen=0;
					break;
				case SDL_MOUSEBUTTONDOWN:
					if (event.button.button == SDL_BUTTON_LEFT) {
						selectIndex = getSelectTile(renderer);
						if (selectIndex == -1) {
							continue;
						}
						selectPiece = game.pieces[game.board[selectIndex]];
						if (selectPiece.color != game.turn || selectPiece.type == NONE_P) {
							selectIndex = -1;
						}
						if (selectIndex != -1) {
							//game.board[selectIndex] = emptyTile;
						}
					}
					break;
				case SDL_MOUSEBUTTONUP:
					if (event.button.button == SDL_BUTTON_LEFT && selectIndex != -1) {
						int index = getSelectTile(renderer);
						if (doMove(selectIndex, index, &game, 0)) {
							if (game.turn == WHITE) {
								game.clock++;
							}
							int prom = isPromotion(&game);
							if (prom > 0) {
								promote(&game, prom, 3);
							}
						}
					}
					selectIndex = -1;

					break;
			}
		}
		graphics_tick++;

		if (graphics_tick == GRAPHIC_UPDATE) {
			graphics_tick = 0;
			SDL_RenderClear(renderer);
			//drawBoard(renderer, BOARD_WIDTH, BOARD_HEIGHT, game);
			// TODO update game's graphics
			if (selectIndex != -1) {
				int x, y;
				int offset = tileSize(renderer)/2;
				SDL_GetMouseState(&x, &y);
				drawPiece(renderer, x-offset, y-offset, pieceToInt(selectPiece));
			}
			SDL_RenderPresent(renderer);
		}

		SDL_Delay(1000/TICK_RATE);
	}

	return 0;
}

int getIndexAtCoords(int x, int y) {

	int tile_size = tileSize(renderer);
	int x_board = x / tile_size;
	int y_board = y / tile_size;

	return x_board + BOARD_WIDTH * y_board;
}

int getSelectTile() {
	int x, y;
	SDL_GetMouseState(&x, &y);
	int index = getIndexAtCoords(x, y);
	return index;
}

# include <ctype.h>
# include <SDL2/SDL.h>
# include "game.h"
# include "graphics.h"
# include "ia.h"
# include "tools.h"
# include "gamestate.h"

const tile emptyTile = { .color=WHITE, .type=NONE_P };
int DEPTH = 6;
SDL_Renderer* renderer;


int cli(void) {
	char command[1000];
	gamestate game;
	int run = 1;
	createAttackMask();
	createPositionMask();
	while(run) {
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




/*
int isMoveValid2(int index, int startIndex, gamestate* game) {
	// Things to check :
	//  - implement underpromotion
	int endX = index % BOARD_WIDTH;
	int endY = index / BOARD_WIDTH;

	int startX = startIndex % BOARD_WIDTH;
	int startY = startIndex / BOARD_WIDTH;

	int pawnDir = -1;
	int oppCol = BLACK;
	int startPawn = 6;
	int kingStartX = 4;
	int kingStartY = 7;
	
	int success = 0;
	int resetEnPassant = 1;

	tile piece = game->board[startIndex];
	tile endTile = game->board[index];

	if (piece.color == BLACK) {
		pawnDir = 1;
		oppCol = WHITE;
		startPawn = 1;
		kingStartY = 0;
	}

	if ( endX == startX && endY == startY) {
		goto end;
	}

	else if ( startIndex < 0 || startIndex >= BOARD_WIDTH * BOARD_HEIGHT ) {
		goto end;
	}
	else if ( index < 0 || index >= BOARD_WIDTH * BOARD_HEIGHT ) {
		goto end;
	}
	else if ( game->board[index].type != NONE_P 
			&& game->board[index].color == piece.color ) {
		goto end;
	}
	int threats[BOARD_HEIGHT*BOARD_WIDTH]={0};
	// the move is executed before we look for check
	gamestate tmp = *game;
	tmp.board[startIndex] = emptyTile;
	tmp.board[index] = piece;
	int kingIndex = 0;
	while(tmp.board[kingIndex].type != KING || tmp.board[kingIndex].color != game->turn) {
		kingIndex++;
	}
	int nbThreats;
	// threatened returns the number of tiles and their position that can attack the target
	nbThreats = threatened(tmp, kingIndex, threats);
	//printThreats2(threats, nbThreats);

	game->board[startIndex] = emptyTile;


	// Pawn moves with en passant, missing promotion
	if (piece.type == PAWN) {
		if ( (endY - startY) * pawnDir  > 0) {
			int indexPassant = index(endX, endY-pawnDir);
			if (startY == startPawn && abs(endY - startY) == 2 && endX == startX
					&& endTile.type == NONE_P
					&& checkObstruction(game, startX, startY, endX, endY, 0, signe(endY - startY))) {
				game->enPassantCandidate = indexPassant;
				resetEnPassant = 0;
				success = 1;
			} else if (endY-startY == pawnDir && endX == startX 
					&& endTile.type == NONE_P) {
				success = 1;
			} else if (endY-startY == pawnDir && abs(endX - startX) == 1 
					&& ((endTile.type != NONE_P 
					&& endTile.color == oppCol))) { 
				success = 1;
			} else if (endY - startY == pawnDir && index == game->enPassantCandidate
					&& game->board[indexPassant].type == PAWN) {
				tile pawn = game->board[indexPassant];
				game->board[indexPassant] = emptyTile;
				int ignore[BOARD_HEIGHT*BOARD_WIDTH];
				int nbThreats2 = threatened(*game, kingIndex, ignore);
				if (nbThreats2) {
					game->board[index(endX, endY - pawnDir)] = pawn;
					success = 0;
				}
				else {
					nbThreats = nbThreats2;	// the check could be resolved thanks to the capture en passant
					success = 1;
				}
			}
		}
	}

	// Knight move
	else if (piece.type == KNIGHT) {
		if ( (abs(endX - startX) == 1 && abs(endY - startY) == 2)
			|| (abs(endX - startX) == 2 && abs(endY - startY) == 1)) {
			success = 1;
		}
	}
	
	// Rook move
	else if (piece.type == ROOK) {
		int dx = endX - startX;
		int dy = endY - startY;
		int sdx = signe(dx);
		int sdy = signe(dy);

		if ( (dx == 0 || dy == 0)
				&& checkObstruction(game, startX, startY, endX, endY, sdx, sdy) 
				&& (endTile.type == NONE_P || endTile.color == oppCol)) {

			// Disabling the castlings if the rooks are moved
			updateCastling(game, piece, startX, startY);
			success = 1;
		}
	}

	// Bishop move
	else if (piece.type == BISHOP) {
		int dx = endX - startX;
		int dy = endY - startY;
		int sdx = signe(dx);
		int sdy = signe(dy);

		if (abs(dx) == abs(dy)
				&& checkObstruction(game, startX, startY, endX, endY, sdx, sdy) 
				&& (endTile.type == NONE_P || endTile.color == oppCol)) {

			success = 1;
		}
	}

	// Queen move
	else if (piece.type == QUEEN) {
		int dx = endX - startX;
		int dy = endY - startY;
		int sdx = signe(dx);
		int sdy = signe(dy);


		if ( ( (dx == 0 || dy == 0) || (abs(dx) == abs(dy)) )
				&& checkObstruction(game, startX, startY, endX, endY, sdx, sdy) 
				&& (endTile.type == NONE_P || endTile.color == oppCol)) {
			success = 1;
		}
	}

	// King move
	else if (piece.type == KING) {
		int dx = endX - startX;
		int dy = endY - startY;
		int sdx = signe(dx);
		int sdy = signe(dy);


		if (abs(dx) <= 1 && abs(dy) <= 1) {
			success = 1;
		}
		if (startX == kingStartX && startY == kingStartY
				&& endX == 6 && endY == startY
				&& game->availableCastling[2*piece.color]
				&& checkObstruction(game, startX, startY, endX, endY, sdx, sdy)) {
			int ignore[BOARD_HEIGHT*BOARD_WIDTH];
			if (threatened(*game, startIndex, ignore) || threatened(*game, startIndex+1, ignore) 
					|| threatened(*game, startIndex +2, ignore)) {
				goto end;
			}
			game->board[startIndex+1] = game->board[index(7, startY)];
			game->board[index(7, startY)] = emptyTile;
			success = 1;
		} else if (startX == kingStartX && startY == kingStartY 
				&& endX == 2 && endY == startY 
				&& game->availableCastling[2*piece.color +1]
				&& checkObstruction(game, startX, startY, 0, endY, sdx, sdy)) { // b-row tile must be empty
			int ignore[BOARD_HEIGHT*BOARD_WIDTH];
			if (threatened(*game, startIndex, ignore) || threatened(*game, startIndex-1, ignore) 
					|| threatened(*game, startIndex -2, ignore)) {
				goto end;
			}
			game->board[startIndex-1] = game->board[index(0, startY)];
			game->board[index(0, startY)] = emptyTile;
			success = 1;
		}
		
		if (!success) {
			goto end;
		}

		// Disabling castlings if the king moves
		updateCastling(game, piece, startX, startY);
	}

end:
	if (success) {
		// if the move is pseudolegal, check if it is legal	
		if (nbThreats > 0) {
			success = 0;
			goto end;
		}
		if (endTile.type != NONE_P) {
			updateCastling(game, game->board[index], endX, endY);
			game->halfmoves = 0;
		} else {
			game->halfmoves++;
		}
		game->board[index] = piece;
		if (resetEnPassant) {
			game->enPassantCandidate = -1;
		}
	} else {
		game->board[startIndex] = piece;
	}
	return success;
}


void updateCastling2(gamestate* game, tile piece, int x, int y) {
	switch (piece.type) {
		case ROOK:
			if (x == 0 && y == 0) {
				game->availableCastling[3] = 0;
			} else if (x == 7 && y == 0) {
				game->availableCastling[2] = 0;
			} else if (x == 0 && y == 7) {
				game->availableCastling[1] = 0;
			} else if (x == 7 && y == 7) {
				game->availableCastling[0] = 0;
			}
			break;
		case KING:
			if (piece.color == WHITE) {
				game->availableCastling[0] = 0;
				game->availableCastling[1] = 0;
			} else {
				game->availableCastling[2] = 0;
				game->availableCastling[3] = 0;
			}
			break;
		default:
			break;
	}
}

*/

# include "ia.h"
# include "tools.h"
# include <stdio.h>
# include <stdlib.h>
# include <ctype.h>
# include "gamestate.h"
# include "bitboard.h"

void printThreats(int threats[]) {
	for ( int j = 0 ; j < BOARD_HEIGHT ; j++ ) {
		for ( int i = 0 ; i < BOARD_WIDTH ; i++ ) {
			printf("%d ", threats[index(i, j)]);
		}
		printf("\n");
	}
	printf("\n");
}

void printThreats2(int threats[], int size) {
	int toPrint[64] = {0};
	for (int i = 0; i < size ; i++) {
		toPrint[threats[i]] = 1;
	}
	printThreats(toPrint);
}

char intToPiece(int type) {
	switch (type) {
		case 0:
			return 'k';
		case 1:
			return 'q';
		case 2:
			return 'b';
		case 3:
			return 'n';
		case 4:
			return 'r';
		case 5:
			return 'p';
		default:
			return ' ';
	}
}

void printBoard(gamestate game) {

	printf("\n");
	for ( int j = 0 ; j < BOARD_HEIGHT ; j++ ) {
		printf(" ");
		for ( int i = 0 ; i < BOARD_WIDTH ; i++ ) {
			printf("+---");
		}
		printf("+\n");
		printf(" ");
		for (int i = 0; i < BOARD_WIDTH ; i++) {
			int piece = game.board[index(i,j)];
			char type = intToPiece(game.pieces[piece].type);
			if (game.pieces[piece].color == WHITE) {
				type = toupper(type);
			}
			printf("| %c ", type);
		}
		printf("| %d\n", BOARD_HEIGHT - j);
	}
	printf(" ");
	for ( int i = 0 ; i < BOARD_WIDTH ; i++ ) {
		printf("+---");
	}
	printf("+\n");
	printf(" ");
	for ( int i = 0 ; i < BOARD_WIDTH ; i++ ) {
		printf("  %c ", 'a'+i);
	}
	printf("\n\n");
}



void indexToName(int index, char result[3]) {
	int x = index % BOARD_WIDTH;
	int y = index / BOARD_WIDTH;
	result[0] = 'a' + x;
	result[1] = '0' + BOARD_HEIGHT - y;
	result[2] = '\0';
}

int nameToIndex(const char name[3]) {
	int x = name[0] - 'a';
	int y = BOARD_HEIGHT + '0' - name[1];
	return index(x, y);
}

void parseConfig(const char command[], gamestate* game) {
	int i = 0;
	// the command starts with "position fen"
	while( command[i++] != ' ');// goes to fen
	while( command[i++] != ' ');// goes to the start of the config
	loadConfiguration(&(command[i]), game);
	//printf("%s\n", command+i);
	while ( command[i] != '\0' && command[i] != 's' ) { // find the beginning of the move list
		i++;
	}
	if (command[i++] != 's') {
		return;
	}
	while (command[i] != '\0') {
		while (command[i] == ' '){
			i++;
		}
		doMove(nameToIndex(&command[i]), nameToIndex(&command[i+2]), game, 0);
		i += 4;
		if (command[i] != ' ' && command[i] != '\0') {	// there is a promotion
			int target = nameToIndex(&command[i-2]);
			int type = 1;
			switch (command[i]) {
				case 'q':
					type = 1;
					break;
				case 'b':
					type = 2;
					break;
				case 'n':
					type = 3;
					break;
				case 'r':
					type = 4;
					break;
			}
			promote(game, target, type);
		}
	}
}

void parseSearch(const char command[], gamestate* game) {
	int i = 0;
	extern int DEPTH;
	while ( !isdigit(command[i++]) );
	i--;
	DEPTH = strtol(&command[i], NULL, 10);
	printf("\nNodes explored : %d\n", depthSearch(*game, DEPTH));
}

int startWith(const char shorter[], const char longer[]) {
	int i = 0;
	while(shorter[i] != '\0' && longer[i] != '\0' && shorter[i] == longer[i]) {
		i++;
	}
	return shorter[i] == '\0';
}

int signe(int x) {
	return (x > 0) - (x < 0);
}

unsigned int get_type(char c) {
	switch (toupper(c)) {
		case 'K':
			return KING;
		case 'Q':
			return QUEEN;
		case 'B':
			return BISHOP;
		case 'N':
			return KNIGHT;
		case 'R':
			return ROOK;
		case 'P':
			return PAWN;
	}
	return 127;
}

void loadConfiguration(const char config[], gamestate* game) {
	// load a null-terminated configuration using the Forsyth-Edwards Notation

	int board_pos = 0;
	int i = 0;
	int lastIndex[2] = {0, NB_PIECES/2};	// last used index for each color
	game->bits = 0;
	game->byColor[WHITE] = 0;
	game->byColor[BLACK] = 0;
	for (int j = 0 ; j < NB_PIECES + 1 ; j++ ) {
		game->pieces[j].type = NONE_P;
		game->pieces[j].color = WHITE;
		game->pieces[j].position = -1;
	}
	for (int j = 0 ; j < BOARD_SIZE ; j++) {
		game->board[j] = NONE_P;
	}

	
	for (i=0; config[i] != '\0' && config[i] != ' '; i++) {
		if (config[i] == '/')
			continue;
		if (isdigit(config[i])) {
			for (int j = 0; j < config[i]-'0'; j++) {
				game->board[board_pos] = NONE_P;
				board_pos++;
			}
			continue;
		}
		int color = WHITE;
		if (isupper(config[i])) {
			color = WHITE;
		}
		else if (islower(config[i])) {
			color = BLACK;
		}
		tile curr = { .color=color, .type=get_type(config[i]), .position=board_pos};
		game->pieces[lastIndex[color]] = curr;
		setAtIndex(&game->byColor[color], board_pos);
		game->board[board_pos] = lastIndex[color];
		lastIndex[color]++;
		if (curr.type == KING) {
			game->kingsIndex[curr.color] = board_pos;
		}
		board_pos++;
	}
	game->bits = game->byColor[WHITE] | game->byColor[BLACK];
	i++;
	if (toupper(config[i]) == 'B') {
		game->turn = BLACK;
	} else {
		game->turn = WHITE;
	}
	i+=2;
	for (int j = 0; j < 4; j++) {
		game->castling[j] = 0;
	}
	for (; config[i] != ' '; i++) {
		switch (config[i]) {
			case 'K':
				game->castling[0] = 1;
				break;
			case 'Q':
				game->castling[1] = 1;
				break;
			case 'k':
				game->castling[2] = 1;
				break;
			case 'q':
				game->castling[3] = 1;
				break;
			case '-':
				for (int j = 0; j < 4; j++) {
					game->castling[j] = 0;
				}
				break;
		}
	}
	i++;
	game->enPassantTarget = -1;
	if (config[i] !=  '-') {
		game->enPassantTarget = nameToIndex(&config[i]);
		i += 2;
	}
	char *end = NULL;
	game->halfmoves = (int) strtol(&(config[i]), &end, 10);
	game->clock = (int) strtol(end, NULL, 10);
}

void printConfiguration(gamestate game) {
	for (int j=0; j < BOARD_HEIGHT; j++) {
		for (int i=0; i < BOARD_WIDTH; i++) {
			printf("%d ", game.pieces[game.board[j*BOARD_WIDTH + i]].type);
		}
		printf("\n");
	}
}
void printBits(bitboard bits) {
	for (int j = 0; j < BOARD_HEIGHT; j++) {
		for (int i = 0; i < BOARD_WIDTH; i++) {
			printf("%d ", getAtIndex(bits, index(i, j)));
		}
		printf("\n");
	}
	printf("\n");
}

# ifndef GAME_H
# define GAME_H

# define TICK_RATE 1000 	// nb of iterations of game loop each second
# define LOGIC_UPDATE 1		// nb of iterations of game loop before updating game's logic	
# define GRAPHIC_UPDATE 20	// nb of iteraitons of game loop before updating game's graphics

# include <SDL2/SDL.h>
# include "gamestate.h"


unsigned int get_type(char c);
int mainLoop(SDL_Window* window, SDL_Renderer* r);
int cli(void);

int getIndexAtCoords(int x, int y);
int getSelectTile();

# endif

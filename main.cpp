#include "game.hpp"
#include <SDL/SDL.h>
#include <iostream>
#include <cassert>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace {

SDL_Surface* screen;

bool end=0;
double prevTime;
void loopIter() {
	SDL_Event e;
	while(SDL_PollEvent(&e)) {
		if (e.type==SDL_QUIT) end=1;
		else if (e.type==SDL_KEYDOWN) {
			SDLKey k = e.key.keysym.sym;
			if (k==SDLK_ESCAPE) end=1;
		} else if (e.type==SDL_KEYUP) {
		} else if (e.type==SDL_MOUSEMOTION) {
			int midx = screen->w/2, midy = screen->h/2;
			if (e.motion.x!=midx || e.motion.y!=midy) {
				moveBow(e.motion.xrel, e.motion.yrel);
				SDL_WarpMouse(midx, midy);
			}
		}
	}
	double time = SDL_GetTicks()/1000.;
	updateGameState(time - prevTime);
	prevTime = time;
	drawFrame();
	SDL_GL_SwapBuffers();

	if (end) {
		SDL_Quit();
	}
}

void mainLoop() {
	prevTime = SDL_GetTicks()/1000.;
	initGame();
#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(loopIter, 60, true);
#else
	while(!end) {
		loopIter();
		SDL_Delay(10);
	}
#endif
}

}

int main(/*int argc, char* argv[]*/) {
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
//	atexit(SDL_Quit);
	screen = SDL_SetVideoMode(800, 600, 0, SDL_OPENGL | SDL_RESIZABLE);
	assert(screen);
	mainLoop();
}

#include "game.hpp"
#include <SDL/SDL.h>
#include <iostream>
#include <cassert>
#include "render/GL.hpp"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
using namespace std;

namespace {

const int NOTE_KEYS[] = {
	271,
	266,
	256,
	275,
	274,
	276,
	305,
	281,
	280,
	307};

SDL_Surface* screen;

bool end=0;
double prevTime;

int getNoteKey(SDLKey k) {
	for(size_t i=0; i<sizeof(NOTE_KEYS)/sizeof(NOTE_KEYS[0]); ++i) {
		if (k==NOTE_KEYS[i]) {
			return i;
		}
	}
	return -1;
}
void loopIter() {
	SDL_Event e;
	while(SDL_PollEvent(&e)) {
		if (e.type==SDL_QUIT) end=1;
		else if (e.type==SDL_KEYDOWN) {
			SDLKey k = e.key.keysym.sym;
			if (k==SDLK_ESCAPE) end=1;
//			cout<<"key "<<k<<'\n';
			int note = getNoteKey(k);
			if (note>=0) keyDown(note);
		} else if (e.type==SDL_KEYUP) {
			SDLKey k = e.key.keysym.sym;
			int note = getNoteKey(k);
			if (note>=0) keyUp(note);
		} else if (e.type==SDL_MOUSEMOTION) {
			int midx = screen->w/2, midy = screen->h/2;
			if (e.motion.x!=midx || e.motion.y!=midy) {
				moveBow(e.motion.xrel, e.motion.yrel);
				SDL_WarpMouse(midx, midy);
			}
		} else if (e.type == SDL_VIDEORESIZE) {
			screen = SDL_SetVideoMode(e.resize.w, e.resize.h, 0, SDL_OPENGL | SDL_RESIZABLE);
			glViewport(0,0,e.resize.w,e.resize.h);
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
	screen = SDL_SetVideoMode(1600, 900, 0, SDL_OPENGL | SDL_RESIZABLE);
	assert(screen);
	mainLoop();
}

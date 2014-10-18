#include "game.hpp"
#include "image.hpp"
#include "render/GL.hpp"
#include "sound/wav_reader.hpp"
#include <SDL/SDL.h>
#include <iostream>
#include <cassert>
#include <cstring>
#include <vector>
#include <cmath>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
using namespace std;

extern double volChange;
extern double curVolume;
extern GameMode gameMode;

namespace {

enum MenuState { START, MENU, GAME };
MenuState menuState = START;

GLuint startTex, menuTex;

const int FREQ = 44100;
const int SAMPLES = 512;

vector<short> startMusic;
vector<short> bgMusic;
vector<short> solo;
size_t musicPos;


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
double soloVolume = 1.0;

int getNoteKey(SDLKey k) {
	for(size_t i=0; i<sizeof(NOTE_KEYS)/sizeof(NOTE_KEYS[0]); ++i) {
		if (k==NOTE_KEYS[i]) {
			return i;
		}
	}
	return -1;
}

void handleKey(SDLKey k) {
	if (menuState == START) {
		if (k==SDLK_RETURN) menuState = MENU;
	} else if (menuState == MENU) {
		if(k==SDLK_n){					
			newGame();
			menuState = GAME;
			prevTime = SDL_GetTicks() / 1000.0;
			musicPos = 0;
		}
		if(k==SDLK_h){
			//HIGHSCORES
		}
		if(k==SDLK_q)
			end = 1;
	} else {
		if (k==SDLK_F10) end=1;
//			cout<<"key "<<k<<'\n';
		int note = getNoteKey(k);
		if (note>=0) keyDown(note);
	}
}

void loopIter() {
	SDL_Event e;
	while(SDL_PollEvent(&e)) {
		if (e.type==SDL_QUIT) end=1;
		else if (e.type==SDL_KEYDOWN) {
			handleKey(e.key.keysym.sym);
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
	if (menuState == START) {
		drawImageFrame(startTex);
	} else if (menuState == MENU) {
		drawImageFrame(menuTex);
	} else if (menuState == GAME) {
		double time = SDL_GetTicks()/1000.;
		updateGameState(time - prevTime);
		prevTime = time;
		drawFrame();
	}
	SDL_GL_SwapBuffers();
	{
		SDL_LockAudio();
		soloVolume = curVolume;
		SDL_UnlockAudio();
	}
	if (menuState == GAME && musicPos > bgMusic.size() + 2*FREQ) {
		menuState = START;
		musicPos = 0;
	}

	if (end) {
		SDL_Quit();
	}
}

void mainLoop() {
	prevTime = SDL_GetTicks()/1000.;
	startTex = makeTexture("alku.jpg");
	menuTex = makeTexture("valikko.jpg");
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

void callback(void* udata, Uint8* s, int len)
{
	(void)udata;
	len /= 2;
	Sint16* stream = (Sint16*)s;

	if (menuState==GAME) {
		for(int i=0; i<len && musicPos+i<bgMusic.size(); ++i) {
			stream[i] = bgMusic[musicPos + i];
		}
		for(int i=0; i<len && musicPos+i<solo.size(); ++i) {
			stream[i] += soloVolume * solo[musicPos + i];
		}
	} else {
		for(int i=0; i<len && musicPos+i<startMusic.size(); ++i) {
			stream[i] = startMusic[musicPos + i];
		}
	}
	musicPos += len;
}

SDL_AudioSpec spec = {
	FREQ, // freq
	AUDIO_S16, // format
	1, // channels
	0, // silence
	SAMPLES, // samples
	0, // padding
	0, // size
	callback, // callback
	0 // userdata
};

#if 0
void genMusic() {
	for(int i=0; i<1<<20; ++i) {
		int n = 1 + i/(FREQ/2) % 4;
		short a = 10000 * sin(M_PI * i * 440*n / FREQ);
		bgMusic.push_back(a);
	}
}
#endif

}

int main(int argc, char* argv[]) {
	for(int i=1; i<argc; ++i) {
		string s = argv[i];
		if (s=="-c") {
			volChange = atof(argv[++i]);
		} else if (s=="-h") {
			gameMode = HARD;
		} else if (s=="-i") {
			gameMode = INSANE;
		} else {
			cout<<"Unknown argument "<<s<<'\n';
		}
	}
	startMusic=WavReader::readUncompressedWavFile("sound/alku.wav");
	bgMusic = WavReader::readUncompressedWavFile("sound/tausta.wav");
	solo = WavReader::readUncompressedWavFile("sound/soolo.wav");
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO);
	SDL_ShowCursor(0);
//	atexit(SDL_Quit);
	screen = SDL_SetVideoMode(1600, 900, 0, SDL_OPENGL | SDL_RESIZABLE);
	assert(screen);
//	genMusic();
	SDL_OpenAudio(&spec, 0);
	SDL_PauseAudio(0);
	mainLoop();
}

#include "game.hpp"
#include "image.hpp"
#include "fft.hpp"
#include "render/GL.hpp"
#include "sound/wav_reader.hpp"
#include "random/highscore.hpp"
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
extern HighScore highScore;
extern long score;
extern bool showScoreGet;

extern float fftRes[];

namespace {


enum MenuState { START, MENU, GAME, ENDING, HIGHSCORE };
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

string name;

const string scoreFile = "scores.dat";
size_t nextFFTPos;
const int FFT_UPDATE_INTERVAL = FREQ / 40;

void updateFFT() {
	const int FFT_SIZE = 4096;
	static float samples[FFT_SIZE];
	static float zeros[FFT_SIZE];
	static float rcos[FFT_SIZE];
	static float rsin[FFT_SIZE];
	int i = 0;
	for(; i<FFT_SIZE && musicPos + i < bgMusic.size(); ++i) {
		float s = 0;
		if (musicPos + i < bgMusic.size()) s += bgMusic[musicPos + i];
		if (musicPos + i < solo.size()) s += soloVolume * solo[musicPos + i];
		samples[i] = s;
	}
	for(;i < FFT_SIZE; ++i)
	  samples[i] = 0;
	vector<float> fsin, fcos;
	asd_fft_f(rcos, rsin, samples, zeros, FFT_SIZE);
	for(int i=0, j=0; i<FFT_BUCKETS; ++i) {
		float sum = 0;
		int end = (i+1)*(FFT_SIZE/2)/FFT_BUCKETS;
		int cnt = end - j;
		for(;j < end; ++j) {
			sum += hypot(rcos[j], rsin[j]);
		}
		fftRes[i] = 0.9f*fftRes[i] + 0.1f*sum / (cnt * FFT_SIZE);
	}
}

void clearMusicPos() {
	SDL_LockAudio();
	musicPos = 0;
	nextFFTPos = 0;
	SDL_UnlockAudio();
}

void changeState(MenuState state) {
	menuState = state;
	switch(state) {
		case START:
			clearMusicPos();
			break;
		case MENU:
			break;
		case GAME:
			newGame();
			prevTime = SDL_GetTicks() / 1000.0;
			clearMusicPos();
			break;
		case ENDING:
			name.clear();
			break;
		case HIGHSCORE:
			break;
		default:
			cerr<<"Invalid state "<<state<<'\n';
			break;
	};
}

void handleKey(SDLKey k) {
	if (k==SDLK_F10) end=1;
	if (menuState == START) {
		if (k==SDLK_RETURN) menuState = MENU;
	} else if (menuState == MENU) {
		if(k==SDLK_n){					
			changeState(GAME);
		}
		if(k==SDLK_h){
			menuState = HIGHSCORE;
		}
		if(k==SDLK_q)
			end = 1;
	} else if (menuState == GAME) {
		int note = getNoteKey(k);
		if (note>=0) keyDown(note);
		if (k==SDLK_t) {
			changeState(ENDING);
			name.clear();
		} else if (k==SDLK_u) {
			showScoreGet = !showScoreGet;
		}
	} else if (menuState == ENDING) {
		if (k == SDLK_RETURN) {
			highScore.addPlayer(name, score);
			highScore.writeToFile(scoreFile);
			changeState(START);
		} else if (k>='a' && k<='z') {
			name += k;
		} else if (k==SDLK_BACKSPACE && !name.empty()) {
			name.erase(name.end()-1);
		}
	} else if (menuState == HIGHSCORE) {
		if (k == SDLK_RETURN) {
			changeState(START);
		}
	}
}
int fullscreen = SDL_FULLSCREEN;

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
			screen = SDL_SetVideoMode(e.resize.w, e.resize.h, 0, SDL_OPENGL | SDL_RESIZABLE | fullscreen);
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
	} else if (menuState == ENDING) {
		drawEnding(name);
	} else if (menuState == HIGHSCORE) {
		drawHighScore();
	}
	SDL_GL_SwapBuffers();
	if (menuState == GAME) {
		SDL_LockAudio();
		soloVolume = curVolume;
		size_t pos = musicPos;
		SDL_UnlockAudio();
		while (pos >= nextFFTPos) {
			updateFFT();
			nextFFTPos += FFT_UPDATE_INTERVAL;
		}
	}
	if (menuState == GAME && musicPos > bgMusic.size() + 2*FREQ) {
		changeState(ENDING);
	}

	if (end) {
		SDL_Quit();
	}
}

void mainLoop() {
	prevTime = SDL_GetTicks()/1000.;
	startTex = makeTexture("data/alku.jpg");
	menuTex = makeTexture("data/valikko.jpg");
	highScore.loadFromFile(scoreFile);
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
	} else if (menuState == START || menuState == MENU) {
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
		} else if (s=="-w") {
			fullscreen = 0;
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
	screen = SDL_SetVideoMode(1600, 900, 0, SDL_OPENGL | SDL_RESIZABLE | fullscreen);
	assert(screen);
//	genMusic();
	SDL_OpenAudio(&spec, 0);
	SDL_PauseAudio(0);
	mainLoop();
}

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
#include <fstream>

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
extern bool slowMusic;

extern float fftRes[];

namespace {

GLuint startTex, menuTex;

const int FREQ = 44100;
const int SAMPLES = 512;

vector<short> startMusic;
vector<short> bgMusic[2];
vector<short> solo[2];
vector<short>& getBG() {
	return bgMusic[slowMusic];
}
vector<short>& getSolo() {
	return solo[slowMusic];
}
size_t musicPos;
double musicSyncAbs;
double musicSyncRel;

const int NUM_KEYS = 10;



#pragma pack(push,1)
struct Config {
	int noteKeys[NUM_KEYS] = {
#if 0
		'a','s','d',
//		271,
//		266,
//		256,
		275,
		274,
		276,
		305,
		281,
		280,
		307
#else
		SDLK_F1, SDLK_F2, SDLK_F3, SDLK_F4, SDLK_F5, SDLK_F6, SDLK_F7, SDLK_F8, SDLK_F9, SDLK_F10
#endif
	};
} config;
#pragma pack(pop)
const string CONFIG_FILE = "config.dat";
void readConfig() {
	ifstream in(CONFIG_FILE);
	if (!in) return;
	in.read((char*)&config, sizeof(config));
}
void writeConfig() {
	ofstream out(CONFIG_FILE);
	out.write((char*)&config, sizeof(config));
}

SDL_Surface* screen;

bool end = false;

double soloVolume = 1.0;

int getNoteKey(SDLKey k) {
	for(int i=0; i<NUM_KEYS; ++i) {
		if (k==config.noteKeys[i]) {
			return i;
		}
	}
	return -1;
}

const string scoreFile = "scores.dat";


struct GameState {
	virtual ~GameState() {}
	virtual void render() = 0;
	virtual void update() {};
	virtual void keyDown(SDLKey key) = 0;
	virtual void keyUp(SDLKey) {};
	virtual bool isInGame() const {return false;}

	static void setState(GameState* state) {
		delete curState;
		curState = state;
	}
	static GameState& get() {
		return *curState;
	}

private:
	static GameState* curState;
};
GameState* GameState::curState = nullptr;

struct StartState: GameState {
	virtual void render() override {drawImageFrame(startTex);}
	virtual void keyDown(SDLKey) override;
};

struct HighScoreState: GameState {
	virtual void render() override {drawHighScore();}
	virtual void keyDown(SDLKey) override {
		setState(new StartState());
	}
};
struct EndingState: GameState {
	string name;
	virtual void render() override {drawEnding(name);}
	virtual void keyDown(SDLKey k) override {
		if (k == SDLK_RETURN) {
			highScore.addPlayer(name, score);
			highScore.writeToFile(scoreFile);
			setState(new HighScoreState());
		} else if (k>='a' && k<='z') {
			name += k;
		} else if (k==SDLK_BACKSPACE && !name.empty()) {
			name.erase(name.end()-1);
		}
	}
};

void updateFFT(size_t pos);
const int FFT_UPDATE_INTERVAL = FREQ / 40;

struct InGameState: GameState {
	double prevGameTime = 0;
	double startTime = SDL_GetTicks() / 1000.0;
	size_t nextFFTPos = 0;
	InGameState() {
		newGame();
		SDL_LockAudio();
		SDL_UnlockAudio();
	}
	virtual bool isInGame() const override {return true;}
	virtual void render() override {
		double time0 = SDL_GetTicks()/1000.;
		time0 += musicSyncRel - (musicSyncAbs - startTime);
		double time = time0 - startTime;
		SDL_LockAudio();
		if (time > prevGameTime) {
			updateGameState(time - prevGameTime);
			prevGameTime = time;
		}
		SDL_UnlockAudio();
		drawFrame();
	}
	virtual void update() override {
		SDL_LockAudio();
		soloVolume = curVolume;
		size_t pos = musicPos;
		SDL_UnlockAudio();
		while (pos + FREQ*10/40 >= nextFFTPos) {
			updateFFT(nextFFTPos);
			nextFFTPos += FFT_UPDATE_INTERVAL;
		}
		if (musicPos > getBG().size() + 2*FREQ) {
			setState(new EndingState());
		}
	}
	virtual void keyDown(SDLKey k) override {
		int note = getNoteKey(k);
		if (note>=0) ::keyDown(note);
		if (k==SDLK_t) {
			setState(new EndingState());
		} else if (k==SDLK_u) {
			showScoreGet = !showScoreGet;
		}
	}
	virtual void keyUp(SDLKey k) override {
		int note = getNoteKey(k);
		if (note>=0) ::keyUp(note);
	}
};

struct ConfigKeyState: GameState {
	int keyIndex = 0;
	virtual void render() override {
		drawConfigFrame(keyIndex+1, NUM_KEYS);
	}
	virtual void keyDown(SDLKey k) override {
		config.noteKeys[keyIndex++] = k;
		if (keyIndex == NUM_KEYS) {
			writeConfig();
			setState(new StartState());
		}
	}
};

struct InMenuState: GameState {
	virtual void render() override {drawMenuFrame(menuTex);}
	virtual void keyDown(SDLKey k) override {
		if(k==SDLK_n){
			setState(new InGameState);
		} else if(k==SDLK_h) {
			setState(new HighScoreState);
		} else if(k==SDLK_q) {
			end = true;
		} else if (k==SDLK_s) {
			slowMusic = !slowMusic;
		} else if(k==SDLK_c) {
			setState(new ConfigKeyState);
		}
	}
};
void StartState::keyDown(SDLKey) {
	setState(new InMenuState);
}

string name;


void updateFFT(size_t mpos) {
	const int FFT_SIZE = 4096;
	static float samples[FFT_SIZE];
	static float zeros[FFT_SIZE];
	static float rcos[FFT_SIZE];
	static float rsin[FFT_SIZE];
	int i = 0;
	for(; i<FFT_SIZE && mpos + i < getBG().size(); ++i) {
		float s = 0;
		if (mpos + i < getBG().size()) s += getBG()[mpos + i];
		if (mpos + i < getSolo().size()) s += soloVolume * getSolo()[mpos + i];
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
		const float smooth = 0.95;
		fftRes[i] = smooth*fftRes[i] + (1-smooth)*sum / (cnt * FFT_SIZE);
	}
}

void clearMusicPos() {
	SDL_LockAudio();
	musicPos = 0;
	SDL_UnlockAudio();
}

void handleKey(SDLKey k) {
	if (k==SDLK_F10) end=1;
	GameState::get().keyDown(k);
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
	GameState::get().render();
	SDL_GL_SwapBuffers();
	static bool prevInGame = false;
	bool inGame = GameState::get().isInGame();
	if (inGame != prevInGame) {
		clearMusicPos();
		prevInGame = inGame;
	}
	GameState::get().update();

	if (end) {
		SDL_Quit();
	}
}

void mainLoop() {
	cout<<"loading textures\n";
	startTex = makeTexture("data/alku.jpg");
	menuTex = makeTexture("data/valikko.jpg");
	highScore.loadFromFile(scoreFile);
	cout<<"init game\n";
	initGame();
	cout<<"start loop\n";
#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(loopIter, 0, true);
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

//	memset(s, 0, len);
	int i=0;
	if (GameState::get().isInGame()) {
		for(i=0; i<len && musicPos+i<getBG().size(); ++i) {
			stream[i] = getBG()[musicPos + i];
		}
		for(i=0; i<len && musicPos+i<getSolo().size(); ++i) {
			stream[i] += soloVolume * getSolo()[musicPos + i];
		}
	} else {
		for(i=0; i<len && musicPos+i<startMusic.size(); ++i) {
			stream[i] = startMusic[musicPos + i];
		}
	}
//	for(;i<len;++i)stream[i]=0;
	musicSyncRel = double(musicPos) / FREQ;
	musicSyncAbs = SDL_GetTicks() / 1000.0;
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
#ifndef __EMSCRIPTEN__
	readConfig();
#endif
	bool resoChange = 0;
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
		} else if (s=="-r") {
			resoChange = true;
		} else {
			cout<<"Unknown argument "<<s<<'\n';
		}
	}
	startMusic=readOggVorbis("sound/alku.ogg");
    startMusic.resize(startMusic.size()+48000);
	bgMusic[0] = readOggVorbis("sound/tausta.ogg");
	solo[0] = readOggVorbis("sound/soolo.ogg");
	bgMusic[1] = readOggVorbis("sound/tausta_hidas.ogg");
	solo[1] = readOggVorbis("sound/soolo_hidas.ogg");
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO);
	SDL_ShowCursor(0);
//	atexit(SDL_Quit);
	SDL_Rect** modes = SDL_ListModes(nullptr, SDL_OPENGL | SDL_FULLSCREEN);
	int w=1600, h=900;
	if (resoChange) {
		w=800, h=600;
		if (modes) {
			for(int i=0; modes[i]; ++i) {
				int ww = modes[i]->w, hh = modes[i]->h;
				if (ww*hh > w*h) {
					w = ww;
					h = hh;
				}
			}
		}
	}
	screen = SDL_SetVideoMode(w, h, 0, SDL_OPENGL | SDL_RESIZABLE | fullscreen);
	assert(screen);
#ifdef USE_GLEW
	glewInit();
#endif
//	genMusic();
	SDL_OpenAudio(&spec, 0);
	GameState::setState(new StartState());
	SDL_PauseAudio(0);
	mainLoop();
	return 0;
}

#include "alkuvalikko.hpp"
#include "sound/wav_reader.hpp"
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <vector>

using namespace std;

vector<short> musa;
size_t musicPos;


const int FREQ = 44100;
const int SAMPLES = 512;

void callback(void* udata, Uint8* s, int len)
{
	(void)udata;
	len /= 2;
	int len0 = len;
	int rem = musa.size() - musicPos;
	len = min(len, rem);
	memcpy(s, &musa[musicPos], 2*len);
	memset(s+2*len, 0, 2*(len0-len));
	musicPos += len;

	Sint16* stream = (Sint16*)s;
	for(int i=0; i<len && musicPos+i<musa.size(); ++i) {
		stream[i] += musa[musicPos + i];
	}
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


Alkuvalikko::Alkuvalikko(){}


void Alkuvalikko::run(){
	SDL_Surface* screen = SDL_SetVideoMode(764, 571, 0, SDL_RESIZABLE);
	SDL_Surface* kuva=IMG_Load("alku.jpg");
	musa=WavReader::readUncompressedWavFile("sound/alku.wav");
	SDL_OpenAudio(&spec, 0);
	SDL_PauseAudio(0);

	SDL_Rect alue;
	alue.x=0;
	alue.y=0;


	bool loop=1;
	for(;loop;){
		SDL_Event e;
		while(SDL_PollEvent(&e)){
			if (e.type==SDL_QUIT){
				SDL_Quit();
				return;			
			}
			
			if(e.type==SDL_KEYDOWN){
				if(e.key.keysym.sym==SDLK_RETURN)
					loop=0;
			}
		}
		
		SDL_BlitSurface(kuva, NULL, screen, &alue);
		SDL_UpdateRect(screen, 0, 0, 764, 571);
		SDL_Delay(10);
	}

	kuva=IMG_Load("valikko.jpg");

	loop=1;
	for(;loop;){
		SDL_Event e;
		while(SDL_PollEvent(&e)){
			if (e.type==SDL_QUIT){
				SDL_Quit();
				return;			
			}
			
			if(e.type==SDL_KEYDOWN){
				if(e.key.keysym.sym==SDLK_n){					
					
					//UUSI PELI

				}
				
				if(e.key.keysym.sym==SDLK_h){

					//HIGHSCORES

				}

				if(e.key.keysym.sym==SDLK_q)
					loop=0;
			}
		}
		
		SDL_BlitSurface(kuva, NULL, screen, &alue);
		SDL_UpdateRect(screen, 0, 0, 764, 571);
		SDL_Delay(10);
	}	
	SDL_Quit();
}

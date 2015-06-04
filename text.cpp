#include "text.hpp"
#include "image.hpp"
#include <SDL/SDL_ttf.h>
#include <cassert>
#include <iostream>
using namespace std;

TTF_Font* font;

void initText() {
	TTF_Init();
	font = TTF_OpenFont("data/Vera.ttf", 32);
	if (!font) {
		cout<<"Failure loading font "<<TTF_GetError()<<'\n';
		assert(0);
	}
}

void writeToTexture(const char* text, int* w, int* h) {
	SDL_Color color = {255,255,255,255};
	SDL_Surface* s = TTF_RenderUTF8_Solid(font, text, color);
	if (!s) {
		cout<<"failed drawing text "<<TTF_GetError()<<'\n';
		assert(0);
	}
	*w = s->w;
	*h = s->h;
#ifdef __EMSCRIPTEN__
	SDL_Surface* apu = SDL_CreateRGBSurface(SDL_SWSURFACE, s->w, s->h, 32, 0xff, 0xff00, 0xff0000, 0xff000000);
	SDL_FillRect(apu, 0, 0);
	SDL_BlitSurface(s, 0, apu, 0);
	int status = MySDL_glTexImage2D(apu);
	SDL_FreeSurface(apu);
#else
	int status = MySDL_glTexImage2D(s);
#endif
	assert(status == 0);
	SDL_FreeSurface(s);
}

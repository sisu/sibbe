#include "text.hpp"
#include "image.hpp"
#include <SDL/SDL_ttf.h>
#include <cassert>
#include <iostream>
using namespace std;

TTF_Font* font;

void initText() {
	TTF_Init();
	font = TTF_OpenFont("data/Vera.ttf", 16);
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
	int status = MySDL_glTexImage2D(s);
	assert(status == 0);
	SDL_FreeSurface(s);
}

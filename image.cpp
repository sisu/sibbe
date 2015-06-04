// http://www.ohjelmointiputka.net/koodivinkit/25013-c-sdl-pinta-opengl-tekstuuriksi
/* Tarvitaan SDL:n ja OpenGL:n otsikot.
 * Jos ei toimi, kokeile nimiä SDL/SDL.h ja SDL/SDL_opengl.h */
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include <iostream>
#include <cassert>
using namespace std;

/* Funktio itse. */
int MySDL_glTexImage2D(SDL_Surface *kuva)
{
#ifdef __EMSCRIPTEN__
	int bpp = kuva->format->BytesPerPixel;
	if (bpp == 4) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kuva->w, kuva->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, kuva->pixels);
	} else {
		cout<<"bpp "<<bpp<<' '<<kuva->w<<' '<<kuva->h<<endl;
		assert(0);
	}
	return 0;
#endif

	SDL_Surface *apu;
	/* Helpottaa, jos tavut ovat järjestyksessä RGBA.
	 * Säädetään siis konetyypin mukaan värien bittimaskit
	 * niin, että tavujen järjestys muistissa osuu oikein. */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	const Uint32 rshift = 24, gshift = 16, bshift = 8, ashift = 0;
#else
	const Uint32 rshift = 0, gshift = 8, bshift = 16, ashift = 24;
#endif
	const Uint32
		rmask = 0xff << rshift,
		gmask = 0xff << gshift,
		bmask = 0xff << bshift,
		amask = 0xff << ashift;
	Uint32 *ptr;
	Uint32 kuva_flags;
	Uint8 kuva_alpha = 0;

	/* Tarkistetaan kuva. */
	if (!kuva || !kuva->w || !kuva->h) {
		return -1;
	}

	/* Otetaan talteen arvot, jotka muuttuvat funktion aikana */
	kuva_flags = kuva->flags;
#ifndef __EMSCRIPTEN__
	Uint32 kuva_colorkey;
	kuva_alpha = kuva->format->alpha;
	kuva_colorkey = kuva->format->colorkey;
#endif

	/* Luodaan apupinta halutussa formaatissa (RGBA). */
	apu = SDL_CreateRGBSurface(SDL_SWSURFACE, kuva->w, kuva->h, 32, rmask, gmask, bmask, amask);
	if (!apu) {
		return -1;
	}
	SDL_FillRect(apu, 0, 0);

	/* Poistetaan erityiset läpinäkyvyysasetukset. */
	SDL_SetAlpha(kuva, 0, 0);
	if ((kuva_flags & SDL_SRCALPHA) != 0 && kuva->format->Amask) {
		SDL_SetColorKey(kuva, 0, 0);
	}

	/* OpenGL:n ja SDL:n y-akselit osoittavat eri suuntiin.
	 * Kopioidaan siis kuva pikselirivi kerrallaan ylösalaisin. */
	SDL_Rect r1, r2;
	r1.x = r2.x = 0;
	r1.h = r2.h = 1;
	r1.w = r2.w = kuva->w;
#if 0
	for (r1.y = 0, r2.y = kuva->h - 1; r2.y >= 0; ++r1.y, --r2.y) {
		SDL_BlitSurface(kuva, &r1, apu, &r2);
	}
#else
	SDL_BlitSurface(kuva, 0, apu, 0);
#endif

	/* Koko pinnan alfa-arvo pitää palauttaa erikseen, jos sellainen on. */
	if ((kuva_flags & SDL_SRCALPHA) && !kuva->format->Amask && kuva_alpha != 0xff) {
		for (r1.y = 0; r1.y < apu->h; ++r1.y) {
			ptr = (Uint32*)((Uint8*) apu->pixels + r1.y * apu->pitch);
			for (r1.x = 0; r1.x < apu->w; ++r1.x) {
				if ((ptr[r1.x] & amask) != 0) {
					ptr[r1.x] &= (kuva_alpha << ashift) | ~amask;
				}
			}
		}
	}

	/* Lähetetään kuva OpenGL:lle, tuhotaan apupinta ja palautetaan asetukset. */
	glTexImage2D(GL_TEXTURE_2D, 0, 4, apu->w, apu->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, apu->pixels);
	SDL_FreeSurface(apu);
#ifndef __EMSCRIPTEN__
	SDL_SetAlpha(kuva, kuva_flags, kuva_alpha);
	SDL_SetColorKey(kuva, kuva_flags, kuva_colorkey);
#endif
	return 0;
}

#include <SDL/SDL_image.h>
#include <cassert>
void loadImage(const char* file) {
	cout<<"loading image "<<file<<'\n';
	SDL_Surface* img = IMG_Load(file);
	assert(img);
	cout<<"calling metabolix's code\n";
	int res = MySDL_glTexImage2D(img);
	assert(!res);
	cout<<"freeing image data\n";
	SDL_FreeSurface(img);
}
GLuint makeTexture(const char* file) {
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	loadImage(file);
	return tex;
}

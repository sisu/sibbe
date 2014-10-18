#pragma once
#include <GL/gl.h>
struct SDL_Surface;
int MySDL_glTexImage2D(SDL_Surface *kuva);
void loadImage(const char* file);
GLuint makeTexture(const char* file);

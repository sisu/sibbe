#include "GL.hpp"

#include "Texture.hpp"
#include <vector>

Texture::~Texture() {
	glDeleteTextures(1, &id);
}

void Texture::init()
{
	glGenTextures(1, &id);

	bind();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void Texture::bind()
{
	glBindTexture(GL_TEXTURE_2D, id);
}

void Texture::setData(float *data, int w, int h)
{
	bind();
	std::vector<unsigned char> v(w*h*4);
	for(int i=0; i<w*h; ++i) for(int j=0; j<4; ++j) v[4*i+j]=data[i]*255;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, &v[0]);
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_FLOAT, data);
}

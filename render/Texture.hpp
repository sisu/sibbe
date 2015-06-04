#pragma once

struct Texture {
	Texture(): id(0) {}
	~Texture();

	void init();

	void bind();

	void setData(float *data, int w, int h);

	GLuint id;

};

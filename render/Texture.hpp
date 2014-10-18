#pragma once

struct Texture {

	void init();

	void bind();

	void setData(float *data, int w, int h);

	GLuint id;

};

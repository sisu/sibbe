#pragma once

struct Texture {

	Texture();

	void bind();

	void setData(float *data, int w, int h);

	int id;

};

#pragma once

#include "GL.hpp"

struct Shader {
	Shader(GLenum type);
	~Shader() {remove();}
	Shader(Shader&& s): type(s.type), id(s.id) {
		s.id = 0;
	}
	Shader& operator=(Shader&& s) {
		remove();
		id = s.id;
		s.id = 0;
		return *this;
	}
	void loadFile(const char* file);
	void loadSource(const char* file);
	void remove();

	GLenum type;
	GLuint id;
};

#include "Shader.hpp"
#include "util/Log.hpp"
#include <cstdlib>
LOG_CHANNEL(Shader);

Shader::Shader(GLenum type): type(type), id(0) {}
void Shader::remove() {
	if (id) glDeleteShader(id);
	id = 0;
}

void Shader::loadFile(const char* file) {
	const int MAX_SIZE = 1<<16;
	char buf[MAX_SIZE];
	FILE* f = fopen(file, "r");
	if (!f) {
		LOG<<"Failed reading shader file "<<file;
		abort();
	}
	int len = fread(buf, 1, MAX_SIZE, f);
	buf[len] = 0;
	fclose(f);
	loadSource(buf);
}

void Shader::loadSource(const char* str) {
	CHECK_GL();
	if (!id) id = glCreateShader(type);
	glShaderSource(id, 1, &str, 0);
	glCompileShader(id);

	int status;
	glGetShaderiv(id, GL_COMPILE_STATUS, &status);
	if (!status) {
		GLsizei elen;
		const int MAX_SIZE=1<<16;
		char buf[MAX_SIZE];
		glGetShaderInfoLog(id, MAX_SIZE, &elen, buf);
		LOG<<"Compiling shader failed: "<<buf;
		LOG<<"Shader:\n"<<str;
		abort();
	}
	LOG<<"Compiled shader:\n"<<str;
	CHECK_GL();
}

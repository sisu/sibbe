#ifndef ZARD_PROGRAM_HPP
#define ZARD_PROGRAM_HPP

#include "Shader.hpp"
#include <memory>

struct Program {
	GLuint id;
	Shader vertex, fragment;

	Program();
	Program(Program&& p): id(p.id), vertex(std::move(p.vertex)), fragment(std::move(p.fragment)) {
		p.id = 0;
	}
	~Program() {remove();}
	Program& operator=(Program&& p) {
		remove();
		id = p.id;
		p.id = 0;
		vertex = std::move(p.vertex);
		fragment = std::move(p.fragment);
		return *this;
	}
	Program(const Program& p) = delete;

	void load();
	void remove();

	static Program fromFiles(const char* sfile, const char* ffile);
};
typedef std::shared_ptr<Program> ProgramPtr;

#endif

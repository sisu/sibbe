#include "Buffer.hpp"
#include <util/Log.hpp>
LOG_CHANNEL(Buffer);

Buffer::~Buffer() {
	clear();
}

void Buffer::clear() {
	if (id) glDeleteBuffers(1, &id);
}

void Buffer::load(GLenum usage) {
	bind(0);
	glBufferData(target, index.back(), 0, usage);
	LOG<<"created buffer of size "<<index.back();
	for(size_t i=0; i<ptrs.size(); ++i) {
		LOG<<"writing " << names[i] << " to indices "<<index[i]<<" - "<<index[i+1];
		glBufferSubData(target, index[i], index[i+1]-index[i], ptrs[i]);
	}
	CHECK_GL();
}
void Buffer::bind(GLuint prog) {
	if (!id) {
		LOG<<"calling genbuf";
		glGenBuffers(1, &id);
		LOG<<"generated buf "<<id;
	}
//	LOG<<"bind buffer "<<target<<" to "<<id;
	gl.bindBuffer(target, id);
	CHECK_GL();
	if (!prog) return;

	for(size_t i=0; i<names.size(); ++i) {
		GLint idx = glGetAttribLocation(prog, names[i].c_str());
//		LOG<<"binding "<<names[i]<<" to "<<idx<<" ; "<<sizes[i]<<' '<<index[i];
		if (idx<0) continue;
		glEnableVertexAttribArray(idx);
		// FIXME: assuming floats
		glVertexAttribPointer(idx, sizes[i], GL_FLOAT, 0, 0, (void*)index[i]);
	}
}

#pragma once

#include "GL.hpp"
#include <string>
#include <vector>
using namespace std;

struct Buffer {
	Buffer(Buffer&& model) {
		ptrs = move(model.ptrs);
		names = move(model.names);
		index = move(model.index);
		sizes = move(model.sizes);
		target = model.target;
		id = model.id;
		model.id = 0;
	}
	Buffer& operator=(Buffer&& model) {
		clear();
		ptrs = move(model.ptrs);
		names = move(model.names);
		index = move(model.index);
		sizes = move(model.sizes);
		target = model.target;
		id = model.id;
		model.id = 0;
		return *this;
	}
	Buffer(const Buffer& model) = delete;

	void load(GLenum usage = GL_STATIC_DRAW);
	void bind(GLuint prog);
	void unbind(GLuint prog);

	template<class T>
	void add(const vector<T>& arr, std::string name, int components) {
		ptrs.push_back(&arr[0]);
		index.push_back(index.back() + arr.size() * sizeof(T));
		names.push_back(name);
		sizes.push_back(components);
	}
	int size(int i) { return index[i+1] - index[i]; }

	Buffer(GLenum target): target(target), id(0) { index.push_back(0); }
	~Buffer();

	void clear();

private:
	vector<const void*> ptrs;
	vector<std::string> names;
	vector<size_t> index;
	vector<int> sizes;
	GLenum target;
	GLuint id;
};

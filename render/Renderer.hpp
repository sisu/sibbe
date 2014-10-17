#pragma once

#include "Program.hpp"
#include "RenderObject.hpp"
#include <util/Matrix.hpp>

struct Renderer {
	void clear();
	void add(RenderObject object);
	void flush();

	Matrix4 transform=Identity();
private:
	vector<RenderObject> objects;

	void draw(const RenderObject& o);
};

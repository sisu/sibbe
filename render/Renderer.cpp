#include "Renderer.hpp"
#include "RenderObject.hpp"
#include "util/Log.hpp"
#include <cassert>
#include <algorithm>
LOG_CHANNEL(Renderer);

void Renderer::clear() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
void Renderer::add(RenderObject object) {
	objects.push_back(std::move(object));
}
bool objectCmp(const RenderObject& a, const RenderObject& b) {
	return a.program->id < b.program->id;
}
void Renderer::flush() {
	CHECK_GL();

	std::sort(objects.begin(), objects.end(), objectCmp);
	for(const RenderObject& o: objects) {
		assert(o.program);
		gl.useProgram(o.program->id);
		draw(o);
	}
	objects.clear();
	glFlush();
	CHECK_GL();
}
void Renderer::draw(const RenderObject& o) {
	GLint idx = glGetUniformLocation(o.program->id, "trans");
	if (idx>=0) glUniformMatrix4fv(idx, 1, 0, (transform * o.transform).ptr());
	idx = glGetUniformLocation(o.program->id, "normalM");
	if (idx>=0) {
		Matrix3 normalM;
		for(int i=0; i<3; ++i) for(int j=0; j<3; ++j) normalM[i][j]=o.transform[i][j];
		glUniformMatrix3fv(idx, 1, 0, normalM.ptr());
	}
	CHECK_GL();
	for(const auto& i: o.paramsv3) {
		GLint id = glGetAttribLocation(o.program->id, i.first.c_str());
		if (id>=0) glVertexAttrib3fv(id, &i.second[0]);
		CHECK_GL();
	}

	assert(o.model);
	Model& m = *o.model;
	m.bind(o.program->id);
//	glBindBuffer(GL_ARRAY_BUFFER, 0);
	CHECK_GL();

//	cout<<"drawing "<<m.indices.size()<<'\n';
	glDrawElements(GL_TRIANGLES, m.indices.size(), GL_UNSIGNED_SHORT, 0);
//	glDrawElements(GL_TRIANGLES, m.indices.size(), GL_UNSIGNED_SHORT, &m.indices[0]);
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
//	glDrawArrays(GL_TRIANGLES, 0, 3);
}

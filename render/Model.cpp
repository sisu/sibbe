#include "Model.hpp"
#include "GL.hpp"
#include <util/Log.hpp>

Model::Model(): vbuf(GL_ARRAY_BUFFER), ibuf(GL_ELEMENT_ARRAY_BUFFER)
{
}

void Model::bind(GLuint prog) {
	CHECK_GL();
	vbuf.bind(prog);
	CHECK_GL();
//	LOGI("setting vertex pointer %d\n", vbuf.index[0]);
//	glBufferData(GL_ARRAY_BUFFER, verts.size*sizeof(Vec3), &verts[0], GL_STATIC_DRAW);

	ibuf.bind(0);
}

void Model::load() {
#if 1
//	Log()<<normals;
	for(const auto& i : vattrs) {
		vbuf.add(i.data, i.name, i.size);
	}
	vbuf.load();
#else
	vbuf.bind();
	glBufferData(GL_ARRAY_BUFFER, verts.size*sizeof(Vec3), &verts[0], GL_STATIC_DRAW);
#endif
	CHECK_GL();

//	for(int i=0; i<indices.size; ++i) LOGI("%d %d\n",i,indices[i]);
	ibuf.add(indices,"idx",1);
	ibuf.load();
	CHECK_GL();
}

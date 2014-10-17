#ifndef ZARD_MODEL
#define ZARD_MODEL
#include "util/Vector.hpp"
#include "Buffer.hpp"
#include <memory>
#include <cassert>
#include <map>
#include <string>
#include <vector>
using namespace std;

struct Model {
	Model(Model&& model) = default;
	Model& operator=(Model&& model) = default;
	Model();

	vector<short> indices;

	void addTriangle(short a, short b, short c) {
		indices.push_back(a);
		indices.push_back(b);
		indices.push_back(c);
	}
#if 0
	void addTriangle(Vec3 a, Vec3 b, Vec3 c) {
		assert(!texCoords.size);
		addTriangle(verts.size, verts.size+1, verts.size+2);
		verts.push_back(a);
		verts.push_back(b);
		verts.push_back(c);
		Vec3 normal = normalize(cross(b-a,c-a));
		normals.push_back(normal);
		normals.push_back(normal);
		normals.push_back(normal);
	}
#endif

	void bind(GLuint prog);
	void load();

	template<int N>
	void setAttr(string name, const vector<Vector<float,N>>& arr) {
		setAttr(name, &arr[0], arr.size());
	}
	template<int N>
	void setAttr(string name, const Vector<float,N>* arr, int k) {
		vector<float> v(N*k);
		for(int i=0; i<k; ++i)
			for(int j=0; j<N; ++j)
				v[N*i+j] = arr[i][j];
		vattrs.push_back({name, v, N});
	}

private:
	Buffer vbuf;
	Buffer ibuf;
	struct Attr {
		string name;
		vector<float> data;
		int size;
	};
	vector<Attr> vattrs;
};
typedef shared_ptr<Model> ModelPtr;

#endif

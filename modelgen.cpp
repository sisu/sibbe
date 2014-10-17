#include "modelgen.hpp"

Model makeCube(float size) {
	Model m;
	Vec3 vts[] = {
		{-1,-1,-1},
		{1,-1,-1},
		{1,1,-1},
		{-1,1,-1},
		{-1,-1,1},
		{1,-1,1},
		{1,1,1},
		{-1,1,1},
	};
	int n = sizeof(vts)/sizeof(vts[0]);
	for(int i=0; i<n; ++i) vts[i] = vts[i]*=size;
	m.setAttr("pos", vts, n);
	for(int i=0; i<n; ++i) vts[i] = normalize(vts[i]);
	m.setAttr("normal", vts, n);
	short idx[6][4] = {
		{0,1,2,3},
		{4,5,6,7},
		{0,1,5,4},
		{2,3,7,6},
		{0,4,7,3},
		{1,5,6,2},
	};
	for(int i=0; i<6; ++i) {
		short* s = idx[i];
		m.addTriangle(s[0],s[1],s[2]);
		m.addTriangle(s[0],s[2],s[3]);
	}
	return m;
}
Model makeQuad(float size) {
	Model m;
	Vec2 vts[] = {
		{-1,-1},
		{1,-1},
		{1,1},
		{-1,1},
	};
	for(int i=0; i<4; ++i) vts[i]*=size;
	m.setAttr("pos", vts, 4);
	Vec3 n = {0,0,1};
	Vec3 ns[] = {n,n,n,n};
	m.setAttr("normal", ns, 4);
	m.addTriangle(0,1,2);
	m.addTriangle(0,2,3);
	return m;
}

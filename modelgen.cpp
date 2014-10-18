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
	m.load();
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
	Vec2 tcoord[] = {
		{0,0},
		{1,0},
		{1,1},
		{0,1},
	};
	for(int i=0; i<4; ++i) vts[i]*=size;
	m.setAttr("pos", vts, 4);
	m.setAttr("tcoord", tcoord, 4);
	Vec3 n = {0,0,1};
	Vec3 ns[] = {n,n,n,n};
	m.setAttr("normal", ns, 4);
	m.addTriangle(0,1,2);
	m.addTriangle(0,2,3);
	m.load();
	return m;
}

Model makeCylinder(float rad, float height, int parts) {
	vector<Vec3> vts;
	vector<Vec3> normals;
	Model m;
	for(int i=0; i<parts; ++i) {
		double t = 2*M_PI*i/parts;
		double x = cos(t), y = sin(t);
		vts.emplace_back(rad*x,rad*y,0);
		vts.emplace_back(rad*x,rad*y,height);
		normals.emplace_back(x,y,0);
		normals.emplace_back(x,y,0);
		int low=2*i, low2=2*(i+1)%parts;
		int hi=low+1, hi2=low2+1;
		m.addTriangle(low, hi, low2);
		m.addTriangle(low2, hi, hi2);
	}
	for(int i=1; i+1<parts; ++i) {
		m.addTriangle(0, 2*i, 2*(i+1));
		m.addTriangle(1, 2*i+1, 2*(i+1)+1);
	}
	m.setAttr("pos", vts);
	m.setAttr("normal", vts);
	m.load();
	return m;
}

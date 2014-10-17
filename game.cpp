#include "game.hpp"
#include "modelgen.hpp"
#include "render/Model.hpp"
#include "render/projection.hpp"
#include "render/Renderer.hpp"
#include <iostream>
using namespace std;

namespace {

Renderer render;
ModelPtr stringModel;
ModelPtr markerModel;
ProgramPtr basicProgram;

struct Note {
	double time;
	int note;

	bool operator<(const Note& n) const {
		return time < n.time;
	}
	bool operator<(double t) const {
		return time < t;
	}
};
vector<Note> notes;

double totalTime;

}

void initGame() {
	stringModel = make_shared<Model>(makeCylinder(0.1, 100, 16));
	markerModel = make_shared<Model>(makeCylinder(0.3, 0.3, 16));
//	stringModel = make_shared<Model>(makeCube());
//	stringModel = make_shared<Model>(makeQuad());
	basicProgram = make_shared<Program>(Program::fromFiles("shaders/t.vert", "shaders/t.frag"));
	render.transform = perspectiveM(1.0, 4.0/3.0, 0.1, 1000);

	notes.push_back({0, 0});
	notes.push_back({1, 1});
	notes.push_back({2, 2});
	notes.push_back({3, 3});
	notes.push_back({4, 0});
}

void updateGameState(double dt) {
	totalTime += dt;
}

void drawFrame() {
	render.clear();
	gl.enable(GL_DEPTH_TEST);
//	Matrix4 view = Matrix4(translate(0,3,-20)) * Rotate(-M_PI*.125, 0);
	Matrix4 view = Matrix4(scale(1,1,-1)) * Matrix4(translate(0,-3,5));
//	render.transform = perspectiveM(1.0, 4.0/3.0, 0.1, 1000);
//	render.transform = Identity();
	Vec2 offset[] = {{-1.5,0}, {-0.5,0.3}, {0.5,0.3}, {1.5,0}};
	for(Vec2 off : offset) {
		RenderObject o(stringModel, basicProgram);
//		obj.transform = translate(0,0,-5);
		o.transform = view * translate(off);
		o.paramsv3["color"] = Vec3(1,0.3,0);
		render.add(o);
	}
	for(Note n: notes) {
		RenderObject o(markerModel, basicProgram);
		Vec3 v = {offset[n.note][0], offset[n.note][1], n.time - totalTime + 10};
		o.transform = view * translate(v);
		o.paramsv3["color"] = Vec3(0,0,1);
		render.add(o);
	}
	render.flush();
}

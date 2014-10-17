#include "game.hpp"
#include "modelgen.hpp"
#include "render/Model.hpp"
#include "render/projection.hpp"
#include "render/Renderer.hpp"
#include "util/math.hpp"
#include <iostream>
#include <deque>
using namespace std;

namespace {

const double BOW_LEN = 20;


Renderer render;
ModelPtr stringModel;
ModelPtr markerModel;
ModelPtr bowModel;
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

double bowX=0, bowY=0;

double totalTime;

int getChosenString() {
	if (bowY < -0.2*M_PI) return 0;
	if (bowY < 0) return 1;
	if (bowY < 0.2*M_PI) return 2;
	return 3;
}

}

void initGame() {
	stringModel = make_shared<Model>(makeCylinder(0.1, 100, 16));
	markerModel = make_shared<Model>(makeCylinder(0.3, 0.3, 16));
	bowModel = make_shared<Model>(makeCylinder(0.2, BOW_LEN, 16));
	basicProgram = make_shared<Program>(Program::fromFiles("shaders/t.vert", "shaders/t.frag"));
	render.transform = perspectiveM(1.0, 4.0/3.0, 0.1, 1000);

	for(int i=0; i<100; ++i) {
		notes.push_back({1.0+i, i%4});
	}
}

void updateGameState(double dt) {
	totalTime += dt;
}

void moveBow(double dx, double dy) {
	bowX += 1e-2*dy;
	bowY += 2e-3*dx;
	bowX = clamp(bowX, -.5*BOW_LEN, .5*BOW_LEN);
	double ylim = .25*M_PI;
	bowY = clamp(bowY, -ylim, ylim);
}

void drawFrame() {
	render.clear();
	gl.enable(GL_DEPTH_TEST);
//	gl.disable(GL_CULL_FACE);
//	Matrix4 view = Matrix4(translate(0,3,-20)) * Rotate(-M_PI*.125, 0);
	Matrix4 view = Matrix4(scale(1,1,-1)) * Matrix4(translate(0,-3,5)) * Rotate(M_PI*0.05, 0);
//	render.transform = perspectiveM(1.0, 4.0/3.0, 0.1, 1000);
//	render.transform = Identity();
	Vec2 offset[] = {{-1.5,0}, {-0.5,0.3}, {0.5,0.3}, {1.5,0}};
	int chosen = getChosenString();
	int idx=-1;
	for(Vec2 off : offset) {
		++idx;
		RenderObject o(stringModel, basicProgram);
//		obj.transform = translate(0,0,-5);
		o.transform = view * translate(off);
		if (idx == chosen) {
			o.paramsv3["color"] = Vec3(1,0.4,0.2);
		} else {
			o.paramsv3["color"] = Vec3(0.7,0.2,0);
		}
		render.add(o);
	}
	const double BOW_POS = 3.0;
	for(Note n: notes) {
		RenderObject o(markerModel, basicProgram);
		Vec3 v = {offset[n.note][0], offset[n.note][1], 2.0*(n.time - totalTime + BOW_POS)};
		o.transform = view * translate(v);
		o.paramsv3["color"] = Vec3(0,0,1);
		render.add(o);
	}
	{
		RenderObject o(bowModel, basicProgram);
		Matrix4 rotateY = Rotate(M_PI*0.5, 1);
		Matrix4 moveToMid = translate(.5*BOW_LEN - bowX,0,0);
		Matrix4 rotateZ = Rotate(bowY, 2);
		Matrix4 moveY = translate(0,1.1,3);
		Vec3 v = {.5*BOW_LEN,1.1,BOW_POS};
//		o.transform = view * translate(v) * rotateY;
		o.transform = view * moveY * rotateZ * moveToMid * rotateY;
		o.paramsv3["color"] = Vec3(0.6,0.2,0);
		render.add(o);
	}
	render.flush();
}

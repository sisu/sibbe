#include "game.hpp"
#include "modelgen.hpp"
#include "render/Model.hpp"
#include "render/projection.hpp"
#include "render/Renderer.hpp"
#include <iostream>
using namespace std;

Renderer render;
ModelPtr stringModel;
ProgramPtr basicProgram;

void initGame() {
	stringModel = make_shared<Model>(makeCube());
//	stringModel = make_shared<Model>(makeQuad());
	stringModel->load();
	basicProgram = make_shared<Program>(Program::fromFiles("shaders/t.vert", "shaders/t.frag"));
	render.transform = perspectiveM(1.0, 4.0/3.0, 0.1, 1000);
}

void updateGameState(double dt) {
}

void drawFrame() {
	render.clear();
	gl.enable(GL_DEPTH_TEST);
	Matrix4 view = Matrix4(translate(0,0,-10)) * Rotate(M_PI*.25, 0);
//	render.transform = perspectiveM(1.0, 4.0/3.0, 0.1, 1000);
//	render.transform = Identity();
	{
		RenderObject o(stringModel, basicProgram);
//		obj.transform = translate(0,0,-5);
		o.transform = view;
		o.paramsv3["color"] = Vec3(1,0,0);
		render.add(o);
	}
	render.flush();
}

#include "game.hpp"
#include "color.hpp"
#include "modelgen.hpp"
#include "text.hpp"
#include "render/Model.hpp"
#include "render/projection.hpp"
#include "render/Renderer.hpp"
#include "util/math.hpp"
#include <iostream>
#include <algorithm>
#include <fstream>
using namespace std;

double volChange = 0.4;
double curVolume = 1.0;
double destVolume = 1.0;

namespace {

const double BOW_LEN = 20;
const double NOTE_SPEED = 25.0;
const double SHOW_BEFORE = 20.0;
const double SHOW_AFTER = 1.0;
const double HIT_RANGE = 0.3;


Renderer render;
ModelPtr stringModel;
ModelPtr markerModel;
ModelPtr bowModel;
ModelPtr quadModel;
ProgramPtr basicProgram;
ProgramPtr textProgram;

struct Note {
	double time;
	int note;
	bool done;

	bool operator<(const Note& n) const {
		return time < n.time;
	}
	bool operator<(double t) const {
		return time < t;
	}

	int string() const {
		return max(0, min(3,note / 10));
	}
	int key() const {
		return note % 10;
	}
};
vector<Note> notes;

double bowX=0, bowY=0;

double totalTime;

bool pressedKeys[32];

long long score = 0;

double getDestVolume() {
	auto iter = lower_bound(notes.begin(), notes.end(), totalTime - HIT_RANGE);
	if (iter == notes.begin()) return 1;
	--iter;
	if (!iter->done) return volChange;
	return 1.0;
}

void updateVolume(double dt) {
	destVolume = getDestVolume();
	if (curVolume < destVolume) curVolume = min(destVolume, curVolume + 5.0 * dt);
	else if (curVolume > destVolume) curVolume = max(destVolume, curVolume - 2.0 * dt);
}


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
	quadModel = make_shared<Model>(makeQuad(1.0));
	basicProgram = make_shared<Program>(Program::fromFiles("shaders/t.vert", "shaders/t.frag"));
	textProgram = make_shared<Program>(Program::fromFiles("shaders/text.vert", "shaders/text.frag"));

#if 0
	for(int i=0; i<100; ++i) {
		notes.push_back({1.0+i, i%4*10, false});
	}
#else
	ifstream in("score/sisu.txt");
	double time;
	int note;
	while(in>>time>>note) {
		notes.push_back({2.4 * time, note, false});
	}
#endif

	initText();
}

void updateGameState(double dt) {
	totalTime += dt;

	auto noteEnd = lower_bound(notes.begin(), notes.end(), totalTime + HIT_RANGE);
	int chosen = getChosenString();
	for(auto iter = lower_bound(notes.begin(), notes.end(), totalTime - HIT_RANGE);
			iter != noteEnd; ++iter) {
		Note& n = *iter;
		if (n.done) continue;
		if (n.string() != chosen) continue;
		if (!pressedKeys[n.key()]) continue;
		n.done = true;
		score += 100;
	}
	updateVolume(dt);
}

void moveBow(double dx, double dy) {
	bowX += 1e-2*dy;
	bowY += 2e-3*dx;
	bowX = clamp(bowX, -.5*BOW_LEN, .5*BOW_LEN);
	double ylim = .25*M_PI;
	bowY = clamp(bowY, -ylim, ylim);
}

void keyDown(int key) {
	pressedKeys[key] = 1;
}
void keyUp(int key) {
	pressedKeys[key] = 0;
}

void drawScore() {
	gl.disable(GL_DEPTH_TEST);
	gl.enable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	render.transform = orthoM(-1, 1, -1, 1, -1, 1);


#if 1
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int w,h;
	char buf[64];
	sprintf(buf, "%lld", score);
	writeToTexture(buf, &w, &h);
#endif
#if 1
	{
		RenderObject o(quadModel, textProgram);
		double hh = 0.1;
		double ww = hh*w/h;
		Matrix4 move = translate(1-ww, 1-hh);
		o.transform = move * scale(ww, hh);
		o.uniform1i["texture"] = 0;
		render.add(o);
	}
#endif
	render.flush();

	glDeleteTextures(1, &tex);
}

void drawFrame() {
	render.clear();
	gl.enable(GL_DEPTH_TEST);
	gl.disable(GL_BLEND);
	render.transform = perspectiveM(1.0, 4.0/3.0, 0.1, 1000);
//	gl.disable(GL_CULL_FACE);
//	Matrix4 view = Matrix4(translate(0,3,-20)) * Rotate(-M_PI*.125, 0);
	Matrix4 view = Matrix4(scale(1,1,-1)) * Matrix4(translate(0,-3,5)) * Rotate(M_PI*0.05, 0);
//	render.transform = perspectiveM(1.0, 4.0/3.0, 0.1, 1000);
//	render.transform = Identity();
//	Vec2 offset[] = {{-1.5,0}, {-0.5,0.3}, {0.5,0.3}, {1.5,0}};
	Vec2 offset[] = {{-1.5,0}, {-0.5,0.0}, {0.5,0.0}, {1.5,0}};
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
	auto noteEnd = lower_bound(notes.begin(), notes.end(), totalTime + SHOW_BEFORE * NOTE_SPEED);
	for(auto iter = lower_bound(notes.begin(), notes.end(), totalTime - SHOW_AFTER * NOTE_SPEED);
			iter != noteEnd; ++iter) {
		Note& n = *iter;
		if (n.done) continue;
		RenderObject o(markerModel, basicProgram);
		Vec2 off = offset[n.string()];
		Vec3 v = {off[0], off[1], NOTE_SPEED*(n.time - totalTime) + BOW_POS};
		o.transform = view * translate(v);
		o.paramsv3["color"] = HSV(n.key() / 10.0, 1.0, 1.0);
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

	drawScore();
}

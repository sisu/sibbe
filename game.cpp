#include "render/GL.hpp"

#include "game.hpp"
#include "color.hpp"
#include "modelgen.hpp"
#include "text.hpp"
#include "image.hpp"
#include "render/Model.hpp"
#include "render/projection.hpp"
#include "render/Renderer.hpp"
#include "util/math.hpp"
#include "random/highscore.hpp"

#include <iostream>
#include <algorithm>
#include <fstream>
using namespace std;

double volChange = 0.4;
double curVolume = 1.0;
double destVolume = 1.0;
GameMode gameMode = HARD;
HighScore highScore;
long long score = 0;
bool showScoreGet = 0;

namespace {

const double BOW_LEN = 20;
const double NOTE_SPEED = 25.0;
const double SHOW_BEFORE = 100.0;
const double SHOW_AFTER = 5.0;
const double HIT_RANGE = 0.3;


Renderer render;
ModelPtr stringModel;
ModelPtr markerModel;
ModelPtr bowModel;
ModelPtr quadModel;
ProgramPtr basicProgram;
ProgramPtr markerProgram;
ProgramPtr textProgram;
GLuint scoreTexture;

ProgramPtr bgProgram;


struct Note {
	Note(double time, int note):time(time), note(note), done(0), score(0) {}
	double time;
	int note;
	bool done;
	bool score;

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

double randf() {
	return (double)rand() / RAND_MAX;
}
struct ScoreShow {
	ScoreShow() {
		pos = Vec2(0.7f,0.5f) + 0.3f*Vec2(randf(), randf());
		dir = Vec2(0, 0.3);
		time = 0.3;
	}
	Vec2 pos;
	Vec2 dir;
	double time;
};
vector<ScoreShow> scoreShow;

int lastKeyPressed = 0;


struct BG
{
	shared_ptr<Program> program;
	Buffer vBuffer = Buffer(GL_ARRAY_BUFFER);
} bg;


void initBG()
{
	bg.program = make_shared<Program>(Program::fromFiles("shaders/bg.vert", "shaders/bg.frag"));

	vector<Vec2> pos = { {-1,-1}, {1,-1}, {-1,1}, {1,1} };
	vector<Vec2> uv = { {0,0}, {1,0}, {0,1}, {1,1} };

	bg.vBuffer.add(pos, "pos", 2);
	bg.vBuffer.add(uv, "texCoords", 2);
	bg.vBuffer.load();
}


double getDestVolume() {
	auto iter = lower_bound(notes.begin(), notes.end(), totalTime - HIT_RANGE);
	if (iter == notes.begin()) return 1;
	--iter;
	if (!iter->score) return volChange;
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
	markerProgram = make_shared<Program>(Program::fromFiles("shaders/t.vert", "shaders/marker.frag"));
	textProgram = make_shared<Program>(Program::fromFiles("shaders/text.vert", "shaders/text.frag"));
	initBG();
	initText();
	scoreTexture = makeTexture("data/sibbe100mk.jpg");
}

void newGame() {
	notes.clear();
	ifstream in("score/sisu.txt");
	double time;
	int note;
	while(in>>time>>note) {
		notes.emplace_back(2.4 * time, note);
	}
	scoreShow.clear();
	score = 0;
	totalTime = 0;
	bowX = bowY = 0;
}

void updateGameState(double dt) {
	totalTime += dt;
	updateVolume(dt);
	for(size_t i=0; i<scoreShow.size(); ) {
		ScoreShow& ss = scoreShow[i];
		ss.time -= dt;
		if (ss.time<0) {
			scoreShow[i] = scoreShow.back();
			scoreShow.pop_back();
		} else {
			ss.pos += float(dt) * ss.dir;
			++i;
		}
	}
}

void moveBow(double dx, double dy) {
	bowX += 1e-2*dy;
	bowY += 2e-3*dx;
	bowX = clamp(bowX, -.5*BOW_LEN, .5*BOW_LEN);
	double ylim = .25*M_PI;
	bowY = clamp(bowY, -ylim, ylim);
}

int lastOkRealKey = -1;

void keyDown(int key) {
	int lastPressedKeyChange = key - lastKeyPressed;
	lastKeyPressed = key;
	auto noteStart = lower_bound(notes.begin(), notes.end(), totalTime - HIT_RANGE);
	auto noteEnd = lower_bound(notes.begin(), notes.end(), totalTime + HIT_RANGE);
	int chosen = getChosenString();
	for(auto iter = noteStart; iter != noteEnd; ++iter) {
		Note& n = *iter;
		if (n.done) continue;
		if (n.string() != chosen) continue;
		if (gameMode <= HARD) {
			if (iter != notes.begin()) {
				if (lastPressedKeyChange) n.done = true;
				auto prev = iter;
				--prev;
				int change = n.key() - prev->key();
				if (change == 0) {
					if (lastOkRealKey!=n.key() || lastPressedKeyChange) continue;
				} else {
					if (lastPressedKeyChange==0 || (lastPressedKeyChange>0)!=(change>0)) continue;
				}
			}
		} else {
			if (n.key() != key) continue;
		}
		n.done = true;
		n.score = true;
		score += 100;
		if (showScoreGet) scoreShow.emplace_back();
		lastOkRealKey = n.key();

		for(auto i=noteStart; i!=iter; ++i) i->done = true;
		break;
	}
}
void keyUp(int key) {
	(void)key;
}

#if 1
void drawText(const char* str, double height, double x, double y, bool right) {
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
	writeToTexture(str, &w, &h);

#endif
#if 1
	{
		RenderObject o(quadModel, textProgram);
		double width = height*w/h;
//		Matrix4 move = translate(1-ww, 1-hh);
		Matrix4 move = translate(right ? x-width : x+width, y-height);
		o.transform = move * scale(width, height);
		o.uniform1i["texture"] = 0;
		render.add(o);
	}
#endif
	render.flush();

	glDeleteTextures(1, &tex);
}
#endif

void drawScore() {
	char buf[64];
	sprintf(buf, "%lld", score);
	drawText(buf, 0.1, 1.0, 1.0, true);
}

void drawScoreShow() {
	glBindTexture(GL_TEXTURE_2D, scoreTexture);
	render.transform = orthoM(-1, 1, -1, 1, -1, 1);
	for(ScoreShow ss: scoreShow) {
		RenderObject o(quadModel, textProgram);
		Matrix4 move = translate(ss.pos);
		o.transform = move * scale(0.1, 0.05);
		o.uniform1i["texture"] = 0;
		render.add(o);
	}
	render.flush();
}

inline Vec3 interpolate(float part, vector<Vec3> vs) {
	if (part==1) return vs.back();
	int cnt = vs.size()-1;
	int fst = part * cnt;
	int snd = fst+1;
	float start = fst / cnt;
	float x = part - start;
	return (1-x) * vs[fst] + x * vs[snd];
}

void drawBg() {

	gl.disable(GL_DEPTH_TEST);

	gl.useProgram(bg.program->id);

	bg.vBuffer.bind(bg.program->id);
 
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	bg.vBuffer.unbind(bg.program->id);
	CHECK_GL();
}


void drawFrame() {
	render.clear();

	drawBg();

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
	auto noteEnd = lower_bound(notes.begin(), notes.end(), totalTime + SHOW_BEFORE / NOTE_SPEED);
	for(auto iter = lower_bound(notes.begin(), notes.end(), totalTime - SHOW_AFTER / NOTE_SPEED);
			iter != noteEnd; ++iter) {
		Note& n = *iter;
		if (n.score) continue;
		RenderObject o(markerModel, markerProgram);
		Vec2 off = offset[n.string()];
		Vec3 v = {off[0], off[1], NOTE_SPEED*(n.time - totalTime) + BOW_POS};
		o.transform = view * translate(v);
//		o.paramsv3["color"] = HSV(n.key() / 10.0, 1.0, 1.0);
//		float part = n.key() / 9.0f;
//		o.paramsv3["color"] = (1-part) * Vec3(1,0,0) + part * Vec3(0,0,1);
//		o.paramsv3["color"] = interpolate(n.key()/9.0, {{.5,.5,1}, {0,0,1}, {1,0,1}, {1,0,0}, {.5,0,0}});
//		o.paramsv3["color"] = interpolate(n.key()/9.0, {{.5,.5,1}, {1,0,1}, {.5,0,0}});
		o.paramsv3["color"] = interpolate(n.key()/9.0,
				{{0,0,1}, {0,1,1}, {0,1,0}, {1,1,0}, {1,0,0}});
		if (n.done) o.paramsv3["color"] = Vec3(0.3,0,0);
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
	drawScoreShow();
}

void drawImageFrame(GLuint tex) {
	render.clear();
	gl.disable(GL_DEPTH_TEST);
	gl.disable(GL_BLEND);
	glBindTexture(GL_TEXTURE_2D, tex);
	render.transform = orthoM(-1, 1, -1, 1, -1, 1);
	RenderObject o(quadModel, textProgram);
	o.transform = Identity();
	o.uniform1i["texture"] = 0;
	render.add(o);
	render.flush();
}

void drawEnding(const string& name) {
	render.clear();
	gl.disable(GL_DEPTH_TEST);
	gl.enable(GL_BLEND);

	const double size = 0.08;
	char buf[64];
	sprintf(buf, "Final score: %lld", score);
	drawText(buf, size, -0.5, 0.8, false);
	drawText("Please enter your name", size, -0.8, 0.6, false);
	string nameString = ">" + name;
	drawText(nameString.c_str(), size, -0.5, 0.4, false);
}

void drawHighScore() {
	render.clear();
	gl.disable(GL_DEPTH_TEST);
	gl.enable(GL_BLEND);

	const double size = 0.08;
	for(int i=1; i<=10; ++i) {
		char buf[64];
		sprintf(buf, "%lld", highScore.getPointsByRank(i));
		drawText(buf, size, 0.2, 0.8 - 0.1*i, true);
		string name = highScore.getNameByRank(i);
		if (!name.empty()) drawText(name.c_str(), size, -0.7, 0.8 - 0.1*i, false);
	}
}

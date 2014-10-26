#include "render/GL.hpp"

#include "render/Texture.hpp"

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
bool showScoreGet = true;
bool slowMusic = false;

float fftRes[FFT_BUCKETS];

namespace {

const double BOW_LEN = 20;
const double NOTE_SPEED = 25.0;
const double SHOW_BEFORE = 100.0;
const double SHOW_AFTER = 5.0;
const double HIT_RANGE = 0.3;
const double BOW_POS = 3.0;
const double STRING_LEN = 100;


Renderer render;
ModelPtr stringModel;
ModelPtr markerModel;
ModelPtr bowModel;
ModelPtr bowHairModel;
ModelPtr quadModel;
ModelPtr violinModel;
ProgramPtr basicProgram;
ProgramPtr markerProgram;
ProgramPtr textProgram;
GLuint scoreTexture;
Vec3 lightCol;

ProgramPtr bgProgram;
ProgramPtr particleProgram;

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

double bowX=0;

double totalTime;
float performance = 1.f; /// 1 if perfect, zero if crappy

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

const float particleTime = 0.2f;

struct Particle
{
  Vec3 pos;
  Vec3 vel;
  Vec3 col;
  float age;
};

vector<Particle> particles;

struct BG
{
	shared_ptr<Program> program;
	Buffer vBuffer = Buffer(GL_ARRAY_BUFFER);
	Texture fftdata;
} bg;


void initBG()
{
	bg.program = make_shared<Program>(Program::fromFiles("shaders/bg.vert", "shaders/bg.frag"));
	bg.fftdata.init();

	vector<Vec2> pos = { {-1,-1}, {1,-1}, {-1,1}, {1,1} };
	vector<Vec2> uv = { {0,0}, {1,0}, {0,1}, {1,1} };

	bg.vBuffer.add(pos, "pos", 2);
	bg.vBuffer.add(uv, "texCoords", 2);
	bg.vBuffer.load();
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


struct String
{
  void init();

  shared_ptr<Model> stringModel;
  shared_ptr<Program> program;
} violinString;

void String::init()
{
  stringModel = make_shared<Model>(makeCylinder(0.05, STRING_LEN, 16));
  vector<float> spiral;
  for(int i = 0; i < 16; ++i) {
    float base = i/4.f;
    spiral.emplace_back(base);
    spiral.emplace_back(base + 400.f);
  }
  stringModel->setAttr("spiral", spiral);
  stringModel->load();

  program = make_shared<Program>(Program::fromFiles("shaders/string.vert", "shaders/string.frag"));
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

const double BOW_MAX = 2.5;
int getChosenString() {
	if (bowX < -0.75*BOW_MAX) return 0;
	if (bowX < 0) return 1;
	if (bowX < 0.75*BOW_MAX) return 2;
	return 3;
}

}

void updateParticles(double dt)
{
	for(size_t i = 0; i < particles.size();) {
		Particle& p = particles[i];
		p.age += dt;
		if(p.age > particleTime) {
			particles[i] = particles.back();
			particles.pop_back();
			continue;
		}
		float left = particleTime - p.age;
		p.pos += float(dt) * left * p.vel;
		++i;
	}
}

void initGame() {
	markerModel = make_shared<Model>(makeCylinder(0.2, 0.2, 16));
	bowModel = make_shared<Model>(makeCylinder(0.15, BOW_LEN, 16));
	bowHairModel = make_shared<Model>(makeQuad(0.15, .5*BOW_LEN));
	quadModel = make_shared<Model>(makeQuad(1.0, 1.0));
	violinModel = make_shared<Model>(makeViolin());
	basicProgram = make_shared<Program>(Program::fromFiles("shaders/t.vert", "shaders/t.frag"));
	particleProgram = make_shared<Program>(Program::fromFiles("shaders/particle.vert", "shaders/particle.frag"));
	markerProgram = make_shared<Program>(Program::fromFiles("shaders/t.vert", "shaders/marker.frag"));
	textProgram = make_shared<Program>(Program::fromFiles("shaders/text.vert", "shaders/text.frag"));
	initBG();
	initText();
	violinString.init();
	scoreTexture = makeTexture("data/sibbe100mk.jpg");
}

void newGame() {
  performance = 1.f;
	notes.clear();
	particles.clear();
	ifstream in("score/sisu.txt");
	double time;
	int note;
	double speedChange = slowMusic ? 4.8 : 2.4;
	while(in>>time>>note) {
		notes.emplace_back(speedChange * time, note);
	}
	scoreShow.clear();
	score = 0;
	totalTime = 0;
	bowX = 0;
}

void updateMissedNotes() {
	auto noteStart = lower_bound(notes.begin(), notes.end(), totalTime - HIT_RANGE);

  auto it = noteStart;
  while(it != notes.begin()) {
    --it;
		Note& n = *it;
    if(n.done)
      continue;
    else {
      n.done = true;
      /// @todo value of performance should be 'continuous', change most at 1/2000 per frame
      performance = max(performance - 1.f/20.f, 0.f);
    }
  }
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

  updateMissedNotes();


	updateParticles(dt);
}

void createParticles(Vec3 col)
{
	const int n = 50;
	int string = getChosenString();
	Vec3 offset[] = {{-1.5,0.f,0}, {-0.5,0.0f,0}, {0.5,0.0f,0}, {1.5,0.f,0}};

	Vec3 basePos = offset[string];
	basePos[2] += BOW_POS;
	for(int i = 0; i < n; ++i) {
		float angle = 2*M_PI*i/n;
		float vel = 5.f/particleTime * rand() / RAND_MAX;
		Vec3 dir(cos(angle), sin(angle), 0);
		particles.push_back({basePos, vel*dir, col, 0});
	}
}


void moveBow(double dx, double dy) {
	(void)dy;
	bowX += 8e-3*dx;
	bowX = clamp(bowX, -BOW_MAX, BOW_MAX);
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
    performance = min(performance + 1.f/100.f, 1.f);

		score += 100;
		createParticles(interpolate(n.key()/9.0,
				{{0,0,1}, {0,1,1}, {0,1,0}, {1,1,0}, {1,0,0}} ));
		if (showScoreGet) scoreShow.emplace_back();
		lastOkRealKey = n.key();

		for(auto i=noteStart; i!=iter; ++i) {
      i->done = true;
    }
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
		o.transform = move * scale(0.1, 0.07);
		o.uniform1i["texture"] = 0;
		render.add(o);
	}
	render.flush();
}

void drawBg() {

	gl.disable(GL_DEPTH_TEST);

	gl.useProgram(bg.program->id);
	bg.vBuffer.bind(bg.program->id);

	bg.fftdata.setData(fftRes, FFT_BUCKETS, 1);

	float sum = 0;
	for(int i = 0.2f*FFT_BUCKETS; i < 0.6f*FFT_BUCKETS; ++i)
	  sum += fftRes[i];
	sum /= (0.4f*FFT_BUCKETS);

  lightCol = Vec3( sum, 4.0*sum*(1.0-sum), 1.0-sum ) * sum;

	auto idx = glGetUniformLocation(bg.program->id, "fft");
	if(idx >= 0)
	  glUniform1i(idx, 0);
	else
	  cout << "Failed to bind tex" << endl;

	idx = glGetUniformLocation(bg.program->id, "avg");
	if(idx >= 0)
	  glUniform1f(idx, sum);

	idx = glGetUniformLocation(bg.program->id, "precision");
	if(idx >= 0)
	  glUniform1f(idx, performance);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	bg.vBuffer.unbind(bg.program->id);
	CHECK_GL();
}

void drawParticles(Matrix4 view)
{
	gl.disable(GL_DEPTH_TEST);
	gl.enable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

//	Matrix4 view = Matrix4(scale(1,1,-1)) * Matrix4(translate(0,-3,5));
	for(size_t i = 0; i < particles.size(); ++i) {
		RenderObject o(quadModel, particleProgram);

		Particle& p = particles[i];

		Matrix4 move = translate(p.pos);
		o.transform = view*move*scale(0.1666f, 0.2f);
//		o.transform = view*move*scale(0.1, 0.1);
		o.uniform1f["age"] = (particleTime - p.age) / particleTime;
		o.uniformv3["color"] = p.col;
		render.add(o);
	}
	render.flush();
	gl.enable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


void drawFrame() {
	render.clear();

	drawBg();

	gl.enable(GL_DEPTH_TEST);
	gl.enable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
		RenderObject o(violinString.stringModel, violinString.program);
//		obj.transform = translate(0,0,-5);
		float pos = BOW_POS-1;
		o.transform = view * translate(off) * translate(0,0,.5*STRING_LEN+pos);
		if (idx == chosen) {
			o.paramsv3["color"] = Vec3(1,1.0,0.2);
		} else {
			o.paramsv3["color"] = Vec3(0.8,0.8,0.8);
		}
    o.uniformv3["lColor"] = o.paramsv3["color"];
		render.add(o);
		RenderObject o2 = o;
//		double yrot = off[0]<0 ? 0.1 : -0.1;
		double yrot = -0.1*off[0];
		o2.transform = view * translate(off) * translate(0,0,pos) * Rotate(yrot, 1) * Rotate(0.1, 0) * translate(0,0,-.5*STRING_LEN);
		render.add(o2);
    o2.uniformv3["lColor"] = o2.paramsv3["color"];
	}
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
    //o.uniformv3["lColor"] = lightCol; marker
		if (n.done) o.paramsv3["color"] = Vec3(0.3,0,0);
		render.add(o);
	}
	{
		const double H0 = 0.95;
		const double D = 0.15;
		double x = bowX;
		double y = H0 - D*x*x;

		RenderObject o(bowModel, basicProgram);
		Matrix4 rotateY = Rotate(M_PI*0.5, 1);
		Matrix4 moveToMid = translate(x,y,0);
		Matrix4 rotateZ = Rotate(atan(2*D*x), 2);
		Matrix4 moveZ = translate(0,0,BOW_POS);
		o.transform = view * moveZ * moveToMid * rotateZ * rotateY;
		o.paramsv3["color"] = Vec3(0.6,0.2,0);
    o.uniformv3["lColor"] = Vec3(0.6,0.2,0);
		render.add(o);

		RenderObject o2(bowHairModel, basicProgram);
		o2.transform = o.transform * Rotate(0.5*M_PI, 0) * translate(0,0,-.5);
		o2.paramsv3["color"] = Vec3(0.8,0.8,0.8);
    o2.uniformv3["lColor"] = Vec3(0.8,0.8,0.8);
		render.add(o2);
	}
	{
		RenderObject o(violinModel, basicProgram);
		o.transform = view * translate(0,-5,4);
		float c = 0.15;
		o.paramsv3["color"] = Vec3(c,c,c);
    o.uniformv3["lColor"] = lightCol;
		render.add(o);
	}
	render.flush();

	drawParticles(view);


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

void drawMenuFrame(GLuint tex) {
	drawImageFrame(tex);
//	string str = string("Game [S]peed: ") + (slowMusic ? "slow" : "fast");
//	drawText(str.c_str(), 0.04, -.95, .1, false);
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

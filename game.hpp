#pragma once
#include "render/GL.hpp"
#include <string>
using namespace std;

enum GameMode { HARD, INSANE };

void initGame();
void newGame();
void updateGameState(double dt);
void moveBow(double dx, double dy);
void keyDown(int key);
void keyUp(int key);
void drawFrame();
void drawEnding(const string& name);
void drawHighScore();

void drawMenuFrame(GLuint tex);
void drawImageFrame(GLuint tex);
void drawConfigFrame(int index, int max);

static const int FFT_BUCKETS = 1000;

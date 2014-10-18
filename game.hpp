#pragma once
#include <GL/gl.h>
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

void drawImageFrame(GLuint tex);

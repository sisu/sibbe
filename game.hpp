#pragma once

enum GameMode { HARD, INSANE };

void initGame();
void newGame();
void updateGameState(double dt);
void moveBow(double dx, double dy);
void keyDown(int key);
void keyUp(int key);
void drawFrame();

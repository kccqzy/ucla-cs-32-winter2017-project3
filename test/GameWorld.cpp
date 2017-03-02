#include "GameWorld.h"
#include <cstdlib>
#include <string>
using namespace std;

bool GameWorld::getKey(int& value) { return false; }

void GameWorld::playSound(int soundID) {}

void GameWorld::setGameStatText(string text) { printf("GameController setting status text: %s\n", text.c_str()); }

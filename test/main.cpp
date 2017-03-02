#include "GameWorld.h"
#include <fstream>
#include <iostream>
#include <string>
using namespace std;

const string assetDirectory = "Assets";

class GameWorld;

GameWorld* createStudentWorld(string assetDir = "");

void run(int argc, char* argv[], GameWorld* gw, std::string windowTitle) {
    for (int i = 1; i < argc; i++) gw->addParameter(argv[i]);
    {
        int status = gw->init();
        if (status == GWSTATUS_LEVEL_ERROR) {
            printf("Displaying message:\n\t%s\n\t%s\n", "Error in data file!", gw->getError().c_str());
            gw->cleanUp();
            return;
        }
    }

    while (1) {
        int status = gw->move();
        if (status == GWSTATUS_PLAYER_WON) {
            printf("Displaying message:\n\t%s\n\t%s\n", ("Winning Ant: " + gw->getWinnerName() + "!").c_str(),
                   "Press Enter to quit...");
            break;
        } else if (status == GWSTATUS_NO_WINNER) {
            printf("Displaying message:\n\t%s\n\t%s\n", "No winning ant!", "Press Enter to quit...");
            break;
        }
    }
    gw->cleanUp();
    return;
}

int main(int argc, char* argv[]) {
    GameWorld* gw = createStudentWorld(assetDirectory);
    run(argc, argv, gw, "Bugs");
    delete gw;
}
